function main(matlabDomainId, vehicle_id)
    % Get current path
    script_directoy = fileparts([mfilename('fullpath') '.m']);

    % Get dev path
    dev_directory = script_directoy;
    for i=1:4
        last_slash_pos = find(dev_directory == '/', 1, 'last');
        dev_directory = dev_directory(1 : last_slash_pos - 1);
    end

    cpm_build_directory = [dev_directory '/cpm_lib/build/'];
    cpm_lib_directory = [cpm_build_directory 'libcpm.so'];

    % Some of these may not be necessary
    setenv("LD_RUN_PATH", [getenv('LD_RUN_PATH'), [':' cpm_build_directory], ':/usr/local/lib/']);
    setenv("LD_LIBRARY_PATH", [getenv('LD_LIBRARY_PATH'), [':' cpm_build_directory], ':/usr/local/lib/']);
    setenv("LD_PRELOAD", [getenv('LD_PRELOAD'), '/usr/lib/x86_64-linux-gnu/libstdc++.so.6', [':' cpm_lib_directory], ':/usr/local/lib/libfastcdr.so', ':/usr/local/lib/libfastrtps.so']);

    % Initialize data readers/writers...
    common_cpm_functions_path = fullfile( ...
        script_directoy, '../../../../cpm_lib/matlab_mex_bindings/' ...
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
    
    path_points = struct;
    for i = 1:numel(s)
        path_points(i).pose.x = x(i);
        path_points(i).pose.y = y(i);
        path_points(i).pose.yaw = yaw(i);
        path_points(i).s = s(i);
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

        TODO: No binding implemented
        
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
        vehicle_command_path_tracking.header.create_stamp.nanoseconds = ...
            uint64(sample(end).t_now);
        vehicle_command_path_tracking.header.valid_after_stamp.nanoseconds = ...
            uint64(sample(end).t_now + dt_valid_after);
        writer_vehicleCommandPathTracking.write(vehicle_command_path_tracking);
        
        % Check for stop signal
        [~, got_stop] = read_system_trigger(reader_systemTrigger, trigger_stop);
    end
end
