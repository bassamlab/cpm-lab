function main(vehicle_id)
    % Get current path
    script_directoy = fileparts([mfilename('fullpath') '.m']);

    % Initialize data readers/writers...
    common_cpm_functions_path = fullfile( ...
        script_directoy, '/..' ...
    );
    assert(isfolder(common_cpm_functions_path), 'Missing folder "%s".', common_cpm_functions_path);
    addpath(common_cpm_functions_path);
    
    matlabDomainId = 1;
    % CAVE `matlabParticipant`must be stored for RTI DDS somewhere
    %   in the workspace  (so it doesn't get gc'ed)
    [matlabParticipant, reader_vehicleStateList, ~, writer_vehicleCommandPathTracking, reader_systemTrigger, writer_readyStatus, trigger_stop] = init_script(matlabDomainId);

    
    %% Sync start with infrastructure
    % Send ready signal
    % Signal needs to be sent for all assigned vehicle ids
    % Also for simulated time case - period etc are set in Middleware,
    % so timestamp field is meaningless
    disp('Sending ready signal');
    ready_msg = ReadyStatus;
    ready_msg.source_id = strcat('hlc_', num2str(vehicle_id));
    ready_stamp = TimeStamp;
    ready_stamp.nanoseconds = uint64(0);
    ready_msg.next_start_stamp = ready_stamp;
    writer_readyStatus.write(ready_msg);

    % Wait for start or stop signal
    disp('Waiting for start or stop signal');    
    got_stop = false;
    got_start = false;
    while (~got_stop && ~got_start)
        [got_start, got_stop] = read_system_trigger(reader_systemTrigger, trigger_stop);
    end
    

    %% Run the HLC
    % Set reader properties
    reader_vehicleStateList.WaitSet = true;
    reader_vehicleStateList.WaitSetTimeout = 5; % [s]

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
        [sample, ~, sample_count, ~] = reader_vehicleStateList.take();
        if (sample_count > 1)
            warning('Received %d samples, expected 1. Correct middleware period? Missed deadline?', sample_count);
            sample = sample(end); % Use latest sample
        end
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
        vehicle_command_path_tracking.header.create_stamp.nanoseconds = ...
            uint64(sample(end).t_now);
        vehicle_command_path_tracking.header.valid_after_stamp.nanoseconds = ...
            uint64(sample(end).t_now + dt_valid_after);
        writer_vehicleCommandPathTracking.write(vehicle_command_path_tracking);
        
        % Check for stop signal
        [~, got_stop] = read_system_trigger(reader_systemTrigger, trigger_stop);
    end
end
