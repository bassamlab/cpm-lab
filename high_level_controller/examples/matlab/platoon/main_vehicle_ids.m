function main_vehicle_ids(matlabDomainId, varargin)
    % Get current path
    
    clc
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

    vehicle_ids = cell2mat(varargin);

    % Send first ready signal 
    % Signal needs to be sent for all assigned vehicle ids
    % Also for simulated time case - period etc are set in Middleware, so timestamp field is meaningless
    disp('Sending ready signals');
    for i = 1 : length(vehicle_ids)
        ready_msg = ReadyStatus;
        ready_msg.source_id = strcat('hlc_', num2str(vehicle_ids(i)));
        ready_msg.next_start_stamp = uint64(0);
        ready_status_writer(ready_msg, matlabDomainId);
    end

    % Wait for start signal
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

    while(~got_stop)
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
    
        % Call the programs to calculate the trajectories of all HLCs
        if (size(vehicle_ids) > 0)
            for i = 1 : length(vehicle_ids)
                msg_leader = leader(vehicle_ids(i), sample.t_now);
                msg_leader.vehicle_id = uint8(vehicle_ids(i));
                vehicle_command_trajectory_writer(msg_leader, matlabDomainId);
            end
        end

        % Check for stop signal, don't wait infinitely for a msg here
        system_trigger = systemTriggerReader(matlabDomainId);
        if system_trigger.is_valid
            if system_trigger.next_start == stop_symbol
                got_stop = true;
            end
        end
    end

    disp('Finished');
    
    % Clear mex files etc. from system memory
    % Else: The transient local ready signal etc. are still being sent
    clear vehicle_command_trajectory_writer
    clear ready_status_writer
    clear systemTriggerReader.m
    clear vehicleStateListReader.m
end