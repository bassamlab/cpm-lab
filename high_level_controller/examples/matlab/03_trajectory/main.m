function main(matlabDomainId, vehicle_id)
    % Get current path
    script_directory = fileparts([mfilename('fullpath') '.m']);

    % Get dev path
    dev_directory = script_directory;
    for i=1:4
        last_slash_pos = find(dev_directory == '/', 1, 'last');
        dev_directory = dev_directory(1 : last_slash_pos - 1);
    end

    cpm_build_directory = [dev_directory '/cpm_lib/build/'];
    cpm_lib_directory = [cpm_build_directory 'libcpm.so'];

    % Some of these may not be necessary
    setenv("LD_RUN_PATH", [getenv('LD_RUN_PATH'), [':' cpm_build_directory], ':/usr/local/lib/']);
    setenv("LD_LIBRARY_PATH", [getenv('LD_LIBRARY_PATH'), [':' cpm_build_directory], ':/usr/local/lib/']);
    setenv("LD_PRELOAD", [getenv('LD_PRELOAD'), ':/usr/lib/x86_64-linux-gnu/libstdc++.so.6', [':' cpm_lib_directory], ':/usr/local/lib/libfastcdr.so', ':/usr/local/lib/libfastrtps.so']);

    % Initialize data readers/writers...
    common_cpm_functions_path = fullfile( ...
        script_directory, '../../../../cpm_lib/matlab_mex_bindings/' ...
    );
    assert(isfolder(common_cpm_functions_path), 'Missing folder "%s".', common_cpm_functions_path);
    addpath(common_cpm_functions_path);

    matlabDomainId = uint32(matlabDomainId);
    
    % This is now obsolete, the reader etc are set up in the mex files, you just need to call them
    % Files to call: ready_status_writer, vehicle_command_trajectory_writer, systemTriggerReader, vehicleStateListReader
    % [matlabParticipant, reader_vehicleStateList, writer_vehicleCommandTrajectory, ~, reader_systemTrigger, writer_readyStatus, trigger_stop] = init_script(matlabDomainId);
    
    
    %% Sync start with infrastructure
    % Send ready signal
    % Signal needs to be sent for all assigned vehicle ids
    % Also for simulated time case - period etc are set in Middleware,
    % so timestamp field is meaningless
    disp('Sending ready signal');
    ready_msg = ReadyStatus;
    ready_msg.source_id = strcat('hlc_', num2str(vehicle_id));
    ready_msg.next_start_stamp = uint64(0);
    ready_status_writer(ready_msg, matlabDomainId);

    % Wait for start or stop signal
    disp('Waiting for start or stop signal');    
    stop_symbol = uint64((0xffffffffffffffffu64));
    got_stop = false;
    got_start = false;
    while (~got_stop && ~got_start)
        system_trigger = systemTriggerReader(matlabDomainId, true);
        if system_trigger.is_valid
            if system_trigger.next_start == stop_symbol
                got_stop = true;
            else
                got_start = true;
            end
        end
    end
    disp('Done');   

    %% Run the HLC
    % Define reference trajectory
    reference_trajectory_index = 1;
    reference_trajectory_time = 0;
    map_center_x = 2.25;
    map_center_y = 2.0;
    trajectory_px    = [       1,        0,       -1,        0] + map_center_x;
    trajectory_py    = [       0,        1,        0,       -1] + map_center_y;
    trajectory_vx    = [       0,       -1,        0,        1];
    trajectory_vy    = [       1,        0,       -1,        0];
    segment_duration = [pi/2*1e9, pi/2*1e9, pi/2*1e9, pi/2*1e9];
    
    while (~got_stop)
        % Read vehicle states / wait for max. 5 seconds
        sample = vehicleStateListReader(matlabDomainId, uint32(5000));
        if ~sample.is_valid
            disp('No new sample received within 5 seconds, stopping...');
            break;
        end
        % assert(sample.is_valid, 'Received no new samples'); Prevents
        % clear depending on when the stop signal is received, causes 
        % trouble in next runs
        fprintf('Received sample at time: %d\n',sample.t_now);
        
        if (reference_trajectory_time == 0)
            reference_trajectory_time = sample.t_now;
        end
        
        % Create reference trajectory
        t_ahead_nanos = 0;
        i_traj_index = reference_trajectory_index;

        trajectory_points = [];
        plan_ahead_time_nanos = 7000e6;
        while (t_ahead_nanos < plan_ahead_time_nanos)
            the_trajectory_point = TrajectoryPoint;
            the_trajectory_point.t = uint64(reference_trajectory_time + t_ahead_nanos);
            the_trajectory_point.px = trajectory_px(i_traj_index);
            the_trajectory_point.py = trajectory_py(i_traj_index);
            the_trajectory_point.vx = trajectory_vx(i_traj_index);
            the_trajectory_point.vy = trajectory_vy(i_traj_index);
            trajectory_points = [trajectory_points the_trajectory_point];
            t_ahead_nanos = t_ahead_nanos + segment_duration(i_traj_index);
            i_traj_index = mod(i_traj_index, length(segment_duration)) + 1;
        end
            
        % Middleware period and maximum communication delay estimation for valid_after stamp
        dt_period_nanos = uint64(sample.period_ms*1e6);
        dt_max_comm_delay = uint64(100e6);
        if dt_period_nanos >= dt_max_comm_delay
            dt_valid_after = dt_period_nanos;
        else
            dt_valid_after = dt_max_comm_delay;
        end
        % Send the current trajectory point to the vehicle
        vehicle_command_trajectory = VehicleCommandTrajectory;
        vehicle_command_trajectory.vehicle_id = uint8(vehicle_id);
        vehicle_command_trajectory.trajectory_points = trajectory_points;
        vehicle_command_trajectory.create_stamp = ...
            uint64(sample.t_now);
        vehicle_command_trajectory.valid_after_stamp = ...
            uint64(sample.t_now + dt_valid_after);
        
        vehicle_command_trajectory_writer(vehicle_command_trajectory, matlabDomainId);

        % The vehicle always needs a trajectory point at or before the current time,
        % as well as enough trajectory points in the future,
        % to allow some time for the vehicle to receive
        % the message and anticipate the next turn.
        next_reference_trajectory_time = ...
            reference_trajectory_time + segment_duration(reference_trajectory_index);
        while (next_reference_trajectory_time < sample.t_now)
            reference_trajectory_time = ...
                reference_trajectory_time + segment_duration(reference_trajectory_index);
            reference_trajectory_index = ...
                mod(reference_trajectory_index, length(segment_duration)) + 1;
            next_reference_trajectory_time = ...
                reference_trajectory_time + segment_duration(reference_trajectory_index);
        end
        
        % Check for stop signal, don't wait infinitely for a msg here
        system_trigger = systemTriggerReader(matlabDomainId);
        if system_trigger.is_valid
            if system_trigger.next_start == stop_symbol
                got_stop = true;
            end
        end
    end
    
    % Clear mex files etc. from system memory
    % Else: The transient local ready signal etc. are still being sent
    clear vehicle_command_trajectory_writer
    clear ready_status_writer
    clear systemTriggerReader.m
    clear vehicleStateListReader.m
end