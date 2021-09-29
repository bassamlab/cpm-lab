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

    % Type cast domain ID to expected type
    matlabDomainId = uint32(matlabDomainId);
    
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

    % Reference path generation
    map_center_x = 2.25;    % [m]
    map_center_y = 2.0;     % [m]
    x     = [     1,      0,     -1,      0,      1] + map_center_x;   % [m]
    y     = [     0,      1,      0,     -1,      0] + map_center_y;   % [m]
    yaw   = [1*pi/2, 2*pi/2, 3*pi/2,      0, 1*pi/2];                  % [rad]
    s     = [     0, 1*pi/2, 2*pi/2, 3*pi/2, 4*pi/2];                  % [m]
    
    path_points = [];
    for i = 1:numel(s)
        path_point = PathPoint;
        path_point.x = x(i);
        path_point.y = y(i);
        path_point.yaw = yaw(i);
        path_point.s = s(i);

        path_points = [path_points [path_point]];
    end
    
    % Main control loop
    while (~got_stop)
        % Read vehicle states
        sample = vehicleStateListReader(matlabDomainId, uint32(5000));
        if ~sample.is_valid
            disp('No new sample received within 5 seconds, stopping...');
            break;
        end
        % assert(sample_count == 1, 'Received %d samples, expected 1', sample_count);
        fprintf('Received sample at time: %d\n',sample.t_now);
        
        % Middleware period and maximum communication delay estimation for valid_after stamp
        dt_period_nanos = uint64(sample.period_ms*1e6);
        dt_max_comm_delay = uint64(100e6);
        if dt_period_nanos >= dt_max_comm_delay
            dt_valid_after = dt_period_nanos;
        else
            dt_valid_after = dt_max_comm_delay;
        end        
        vehicle_command_path_tracking = VehicleCommandPathTracking;
        vehicle_command_path_tracking.vehicle_id = uint8(vehicle_id);
        vehicle_command_path_tracking.path = path_points;
        vehicle_command_path_tracking.speed = 1.0;
        vehicle_command_path_tracking.create_stamp = ...
            uint64(sample(end).t_now);
        vehicle_command_path_tracking.valid_after_stamp = ...
            uint64(sample(end).t_now + dt_valid_after);
        vehicle_command_path_tracking_writer(vehicle_command_path_tracking, matlabDomainId);
        
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
    clear vehicle_command_path_tracking_writer
    clear ready_status_writer
    clear systemTriggerReader.m
    clear vehicleStateListReader.m
end
