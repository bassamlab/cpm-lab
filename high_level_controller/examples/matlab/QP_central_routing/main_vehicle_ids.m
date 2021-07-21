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

function main_vehicle_ids(varargin)
    % Get current path
    clc
    script_directoy = fileparts([mfilename('fullpath') '.m']);
    cd(script_directory)

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

    matlabDomainId = uint32(1);
    
    
    vehicle_ids = varargin
    numVehicles = length(vehicle_ids); 

    
    %% IBM CPLEX path
    CPLEX_PATH = '/opt/ibm/ILOG/CPLEX_Studio1210/cplex/matlab/x86-64_linux';
    addpath(CPLEX_PATH);
    
    
    %% import CommonRoad path
    addpath('./RTree');
    addpath('./Maps');
    
    %% HLC 
    addpath('./ControllerFiles');
    
   

    %% initialisation procedure
    filepath = './Maps/LabMapCommonRoad.xml'; 
    commonroad_data = LoadXML(filepath);

    
   

    
    %% initialize vehicles 
    
    StartLaneletId = 0*(1:numVehicles); % dummy zeros, because GUI initialize vehicles somewhere random
    
    % flags for Initialization of each vehicle
    VehNotStarted = ones(numVehicles,1);
    activeVehicleList = [];
    bool_isOfflinePhase = uint8(1); % flag for synchronized start
    
    vehicles = cell(1,numVehicles);

    for k = 1:numVehicles
        vehicles{1,k} = vehicle(k,vehicle_ids{k},StartLaneletId(k),commonroad_data.map,commonroad_data.r_tree);
    end
 
    % storage of trajectory data of each vehicle, passed to hlc for collision avoidance
    hlc_trajectoryLastIter = cell(numVehicles,2);

    % Send first ready signal 
    % Signal needs to be sent for all assigned vehicle ids
    % Also for simulated time case - period etc are set in Middleware, so timestamp field is meaningless
    disp('Sending ready signals');
    for i = 1 : length(vehicle_ids)
        ready_msg = ReadyStatus;
        ready_msg.source_id = strcat('hlc_', num2str(vehicle_ids{i}));
        ready_msg.next_start_stamp = uint64(0);
        ready_status_writer(ready_msg, matlabDomainId);
    end
       
    
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
        
    
        % read status to see if position is delivered by middleware, assuming
        % vehicle not starting at (0,0) (initialized with (0,0) by
        % default)
        %if active, initialize hlc of vehicles. 
        if bool_isOfflinePhase == 1
            for k = 1:numVehicles
                if VehNotStarted(k) == 1
                        msg = ['sample.state_list(1,',num2str(k),'): x=',num2str(sample.state_list(1,k).pose.x),'; y= ',num2str(sample.state_list(1,k).pose.y)];
                        disp(msg);
                        msg = ['sample.state_list(1,',num2str(k),'): speed= ',num2str(sample.state_list(1,k).speed)];
                        disp(msg);
                        msg = ['sample.state_list(1,',num2str(k),'): yaw= ',num2str(sample.state_list(1,k).pose.yaw)];
                        disp(msg);
                        
                        % vehicle is active
                        if ~(sample.state_list(1,k).pose.x == 0 && sample.state_list(1,k).pose.y == 0)
                            VehNotStarted(k) = 0; % vehicle is online
                            activeVehicleList = sort([activeVehicleList, k]);         
                        end
                end
            end
            
            % check if all vehicles are active. if yes, go to
            % online phase
            if sum(VehNotStarted) == 0
                bool_isOfflinePhase =0; % go online
                % initialize path planning
                for k = 1:numVehicles
                    vehicles{1,k}.update(sample.state_list(1,k));
                    vehicles{1,k}.resetPathPlanner();
                end
            else
                % not all vehicles are online, skip current loop
                % iteration until all vehicles are online
                continue;
            end
        end

        

        
        
        % Call the programs to calculate the trajectories of all HLCs
        if (size(vehicle_ids) > 0)
                
                for j = 1 : numVehicles
                    
                    k = numVehicles +1 - j;
                    if VehNotStarted(k) == 0
                        % position update and ref traj update
                        vehicles{1,k}.update(sample.state_list(1,k)); % position update
                        vehicles{1,k}.updatePath(sample.t_now); % path update
                        
                        
                        % get active vehicles with higher priority
                        higherPriorityindex = (activeVehicleList>k);
                        higherPriorityVehicles = activeVehicleList(higherPriorityindex);     
                        VehicleCollision =  hlc_trajectoryLastIter(higherPriorityVehicles,:);
                        
                        % no obstacles in cenral routing, only
                        % Vehicles
                        dynamicObstaclesVehicles = VehicleCollision;
                        
                        
                        % calculate trajectory
                        vehicles{k}.optimizeTraj(dynamicObstaclesVehicles);
                        
                        % convert to message
                        [msg, position, time ]= vehicles{1,k}.getTrajMsg();
                        vehicle_command_trajectory_writer(msg);
                        
                        % update collision trajectory.
                        hlc_trajectoryLastIter{k,1} = position;  % update;
                        hlc_trajectoryLastIter{k,2} = time;  % update;
                    end
                    
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
