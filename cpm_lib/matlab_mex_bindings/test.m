% MIT License
% 
% Copyright (c) 2020 Lehrstuhl Informatik 11 - RWTH Aachen University
% 
% Permission is hereby granted, free of charge, to any person obtaining a copy
% of this software and associated documentation files (the "Software"), to deal
% in the Software without restriction, including without limitation the rights
% to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
% copies of the Software, and to permit persons to whom the Software is
% furnished to do so, subject to the following conditions:
% 
% The above copyright notice and this permission notice shall be included in all
% copies or substantial portions of the Software.
% 
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
% IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
% OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
% SOFTWARE.
% 
% This file is part of cpm_lab.
% 
% Author: i11 - Embedded Software, RWTH Aachen University

function main(vehicle_id)
    % Get current path
    clc
    script_directoy = fileparts([mfilename('fullpath') '.m']);

    % Add all libraries to the path
    %setenv("LD_RUN_PATH", [getenv('LD_RUN_PATH'), ':/home/leon/dev/software/cpm_lib/build/', ':/usr/local/lib/']);
    setenv("LD_LIBRARY_PATH", [getenv('LD_LIBRARY_PATH'), ':/home/leon/dev/software/cpm_lib/build/', ':/usr/local/lib/']);
    %setenv("LD_PRELOAD", [getenv('LD_PRELOAD'), ':/usr/lib/x86_64-linux-gnu/libstdc++.so.6', ':/home/leon/dev/software/cpm_lib/build/libcpm.so:/usr/local/lib/libfastcdr.so', ':/usr/local/lib/libfastrtps.so']);
    % disp(getenv('LD_LIBRARY_PATH'));

    %%WARNING: THIS DOES NOT WORK - INSTEAD, CALL THIS BEFORE STARTING MATLAB (replace with your own file locations)
    %export LD_PRELOAD="/usr/lib/x86_64-linux-gnu/libstdc++.so.6:/home/leon/dev/software/cpm_lib/build/libcpm.so:/usr/local/lib/libfastcdr.so::/usr/local/lib/libfastrtps.so"

    % disp('Calling init.');
    % mex_test_eprosima('create');

    % disp('Waiting...');
    % pause(1);

    % disp('Calling write');
    % mex_test_eprosima('write');

    % disp('Calling delete');
    % mex_test_eprosima('delete');

    ready_status = ReadyStatus;
    ready_status.source_id = 'hlc_1';
    ready_status.next_start_stamp = 2021;
    domain_id = uint32(2);

    % Testing the ready signal
    ready_status_writer(ready_status, domain_id);

    % What happens if no system trigger was yet sent?
%     system_trigger = SystemTrigger;
%     system_trigger = system_trigger_reader(system_trigger, true);
%     disp(system_trigger);
    % -> As you can see: I added a value called is_valid to the system trigger, which is false if no msg was received

    % System trigger test
    system_trigger = systemTriggerReader(uint32(2), true);
    
    % Test if the specified data type works    
    state_list = vehicleStateListReader(uint32(2), uint32(5000));

    % Now wait for a msg
    % system_trigger = SystemTrigger;
    % system_trigger = system_trigger_reader(system_trigger, true); OUTDATED, now you 
    % do not need to pass system_trigger anymore and get a struct returned
    % ALSO NOTE: USE systemTriggerReader INSTEAD! (creates a new Matlab Thread for listening)
    % disp(system_trigger);

    % Check the trajectory writer
    % Create a trajectory command object
    trajectory = VehicleCommandTrajectory;
    trajectory.vehicle_id = 1;
    trajectory.create_stamp = 7;
    trajectory.valid_after_stamp = 7;
    
    % Create trajectory points
    point1 = TrajectoryPoint;
    point2 = TrajectoryPoint;
    point3 = TrajectoryPoint;
    point1.px = 0;
    point2.px = 1;
    point3.px = 3;
    
    domain_id = uint32(2);
    
    trajectory.trajectory_points = [point1, point2, point3];
    vehicle_command_trajectory_writer(trajectory, domain_id);
    
    pause(1);

    % Clear mex files etc. from system memory
    % Else: The transient local ready signal etc. are still being sent
    clear vehicle_command_trajectory_writer
    clear ready_status_writer
    clear systemTriggerReader.m
    clear vehicleStateListReader.m
end