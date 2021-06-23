# eProsima for Matlab {#matlab_readme}
Located in cpm_lib/matlab_mex_bindings and referring to files in this folder.

## How to compile these files
You need to compile these files using MEX. This should already be taken care of by the build_mex.bash here. **BUT**: You need to make some preparations beforehand.

In the following, I will assume that you already have installed FastDDS and FastCDR and put their library files in your global installation folder (by installing these globally) at /usr/local/lib.

1. Before you do so, you need to make sure that, besides Matlab, you have a supported GCC version installed on your system (see https://de.mathworks.com/support/requirements/supported-compilers.html). Also, please note that version 2021a of Matlab was used when developing these files, and that older versions might not be supported.

2. Install the newest recommended gcc version (and g++ version). Then use update-alternatives to set the current system compiler to these gcc and g++ versions. 

3. Now you should be able to build the readers and writers (..._reader.cpp, ..._writer.cpp). You need to link your cpm lib file as well as the installed fastdds lib files to make the compilation work - just use the commands in build_mex.bash as an orientation. It is expected that Matlab is installed on your system and that it has been added to your path.
```bash
mex vehicle_command_trajectory_writer.cpp -Lpath_to_cpm_lib_folder/build -lcpm -Ipath_to_cpm_lib_folder/include -L/usr/local/lib -lfastcdr -lfastrtps
```

4. YOU ARE NOT DONE YET!

## How to execute these files
Matlab will not be able to execute your Mex-Files without preparation!

1. You need to preload all required library files **before** you start Matlab. You may notice that libstdc++ is included here as well. I will explain that in step 3. DO NOT set this variable globally unless you want some of your programs to crash on start. (Adapt the folder names depending on your system setup):
```bash
export LD_PRELOAD="/usr/lib/x86_64-linux-gnu/libstdc++.so.6:/home/leon/dev/software/cpm_lib/build/libcpm.so:/usr/local/lib/libfastcdr.so::/usr/local/lib/libfastrtps.so"
```

2. You also again need to include the files in your Matlab file (adapt the folder names depending on your system setup):
```matlab
setenv("LD_LIBRARY_PATH", [getenv('LD_LIBRARY_PATH'), ':/home/leon/dev/software/cpm_lib/build/', ':/usr/local/lib/']);
```

3. At this point, you should already be able to execute the mex files directly. You may wonder why libstdc++ is included as well. Matlab comes with its own C++ Compiler / Environment, which is usually outdated, and depending on your system setup, mex may build files that are **incompatible** with Matlabs C++ version when executing these files in Matlab, resulting in an error. Thus, you either need to set LD_PRELOAD globally in your system (reason explained in the next step) or replace Matlabs own C++ version with a link to your system's version.

4. For the readers to work properly, they are started in a new Matlab thread. This thread seems to ignore your set C++ version in LD_PRELOAD. The execution fails. As a result, you need to implement a workaround:
    - (Not working) Set LD_PRELOAD globally as environment variable -> Other programs may crash & this does not work
    - (Untested) Use the same "outdated" compiler for mex AND to create your library files (libcpm, libfastdds)
    - (Tested, works) Replace Matlabs internal C++ environment with a link to yours (inspired by Stackoverflow):
    ```bash
    cd path_to_matlab/sys/os/glnxa64
    sudo mkdir old
    sudo mv libstdc++.so.6* old
    sudo ln -s /usr/lib/x86_64-linux-gnu/libstdc++.so.6 libstdc++.so.6 
    ```

5. If you made any mistake: Make sure that all readers and writers are cleared, and do that manually (see [next section](#always-make-sure-to-clear-your-objects-after-you-have-used-them)) if necessary. Else, you may be stuck with an error-throwing version (even when it comes to loading the libraries) even after you have fixed the bugs that caused that error.

6. You should now be ready to go. The next section explains how the readers and writers are supposed to be used

## How to use the provided readers and writers
Note: This guide is not final, as it is, in its current version, not fully compatible with remote deployment!

### Importing the readers and writers
Make sure to import this folder in your matlab file. An example can be seen below:

```matlab
% Initialize data readers/writers...
common_cpm_functions_path = fullfile( ...
    script_directoy, '../../../../cpm_lib/matlab_mex_bindings/' ...
);
assert(isfolder(common_cpm_functions_path), 'Missing folder "%s".', common_cpm_functions_path);
addpath(common_cpm_functions_path);
```

### Using the readers
Both readers **should not be used directly**, unless you only want them to receive data while their mex-function is called and before it returns. You very likely don't want that to happen and want the readers to receive data while your code is doing other things as well.

For this purpose, the wrappers systemTriggerReader and vehicleStateListReader have been created. They let the readers run in a new thread.

#### Receiving systemTrigger messages
These messages inform the HLC if it should start (check after initialization) or stop (check in every timestep) its computation. You do not need to interpret the value of the signal - the Middleware takes care of these timing-issues for you. The only thing you need to consider is the value of the stop symbol, which you can define as: 

```matlab
stop_symbol = uint64((0xffffffffffffffffu64));
```

The mex-file **returns** the received system trigger in form of a struct. It contains: 
- *next_start* as a uint64
- *is_valid* as a boolean

If no message was received, **is_valid** is false.

The file also takes an optional **input** parameter:
- *wait_infinitely* as a boolean

If you pass *true*, this means that the reader will wait for up to max_unsigned_int milliseconds before it returns.

Calling the reader thus may look like this:

```matlab
system_trigger = systemTriggerReader(true);
```

Here is a full example:

```matlab
disp('Waiting for start or stop signal');    
stop_symbol = uint64((0xffffffffffffffffu64));
got_stop = false;
got_start = false;
while (~got_stop && ~got_start)
    system_trigger = systemTriggerReader(true);
    if system_trigger.is_valid
        if system_trigger.next_start == stop_symbol
            got_stop = true;
        else
            got_start = true;
        end
    end
end
disp('Done');  
```

#### Receiving vehicleStateList messages
The basic idea is similar to systemTrigger. 

**Input**:
- Optional uint32, specifying the max. time to wait for a message in milliseconds

```matlab
sample = vehicleStateListReader(uint32(5000));
```

The mex-file **returns** the received VehicleStateList in form of a struct. It contains: 
- *t_now* as a uint64
- *period_ms* as a uint64
- *state_list* as a struct array. Each entry contains:
    - *vehicle_id* as a uint8
    - *pose_x* as a double
    - *pose_y* as a double
    - *pose_yaw* as a double
    - *IPS_update_age_nanoseconds* as a uint64
    - *odometer_distance* as a double
    - *imu_acceleration_forward* as a double
    - *imu_acceleration_left* as a double
    - *imu_acceleration_up* as a double
    - *imu_yaw* as a double
    - *imu_yaw_rate* as a double
    - *speed* as a double
    - *battery_voltage* as a double
    - *motor_current* as a double
    - *motor_throttle* as a double
    - *steering_servo* as a double
    - *is_real* as a boolean
- *vehicle_observation_list* as a struct array. Each entry contains:
    - *vehicle_id* as a uint8
    - *pose_x* as a double
    - *pose_y* as a double
    - *pose_yaw* as a double
- *active_vehicle_ids* as a list of int32
- *is_valid* as a boolean

Again, the received data is only valid if **is_valid** is true, else no message was actually received and the rest of the struct should either be empty or default to zero.

### Using the writers
Using the writers is just as simple. To write a message, you first need to create a class object of the message you want to write, ReadyStatus or VehicleCommandTrajectory, then fill it with the information you want to send and finally write it with the correct writer. 

You just need to pass the correct object (type) to the mex-writer to send it.

#### ReadyStatus writer
A full example of creating the ReadyStatus message object, filling it with data and sending it is shown below:

```matlab
ready_msg = ReadyStatus;
ready_msg.source_id = strcat('hlc_', num2str(vehicle_id));
ready_msg.next_start_stamp = uint64(0);
ready_status_writer(ready_msg);
```

#### VehicleCommandTrajectory writer
As before, here is a full example with setting up and sending the message.

```matlab
% Some code was left out

% Set up the trajectory points
trajectory_points = [];

the_trajectory_point = TrajectoryPoint;
the_trajectory_point.t = ...;
the_trajectory_point.px = ...;
the_trajectory_point.py = ...;
the_trajectory_point.vx = ...;
the_trajectory_point.vy = ...;
trajectory_points = [trajectory_points the_trajectory_point];
    
% Set up the vehicle command trajectory message
max_delay_time_nanos = 200e6;
vehicle_command_trajectory = VehicleCommandTrajectory;
vehicle_command_trajectory.vehicle_id = uint8(vehicle_id);
vehicle_command_trajectory.trajectory_points = trajectory_points;
vehicle_command_trajectory.create_stamp = ...
    uint64(sample.t_now);
vehicle_command_trajectory.valid_after_stamp = ...
    uint64(sample.t_now + max_delay_time_nanos);

% Send the message using the mex DDS writer
vehicle_command_trajectory_writer(vehicle_command_trajectory);
```

### Always make sure to clear your objects after you have used them
```matlab
clear vehicle_command_trajectory_writer
clear ready_status_writer
clear systemTriggerReader.m
clear vehicleStateListReader.m
```

If you do not do this, old readers and writers will stay in memory. They may e.g. have old stop messages buffered and will thus let your program end immediately if you start it the next time.