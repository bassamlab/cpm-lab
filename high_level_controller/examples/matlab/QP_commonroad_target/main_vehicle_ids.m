function main_vehicle_ids(matlabDomainId, varargin)
    % Get current path
    clc
    script_directory = fileparts([mfilename('fullpath') '.m']);
    cd(script_directory);

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


    vehicle_ids = varargin
    numVehicles = length(vehicle_ids);
     %% IBM CPLEX path
    CPLEX_PATH = '/opt/ibm/ILOG/CPLEX_Studio1210/cplex/matlab/x86-64_linux';
    addpath(CPLEX_PATH);
    
    
    %% import CommonRoad path
    addpath('./RTree');
    addpath('./Maps');
    filepath = './Maps/LabMapCommonRoad.xml';
    addpath('./CommonRoadImport');
    
    %% HLC 
    addpath('./ControllerFiles');
    
    
    %% initialisation procedure for CommonRoad format
    
    % choose scenario
%     filepath = './Maps/LabMapCommonRoadPlanningObstacle.xml';
%     filepath = './Maps/LabMapCommonRoadPlanning.xml';
    filepath = './Maps/LabMapCommonRoadPlanning2Vehicles.xml';
    
    commonroad_data = LoadXML(filepath);
    numObst = commonroad_data.Obstacle_Data.numObst; % number of obstacles
    numPlanning = commonroad_data.Planning_Data.numPlanning; % number of planning problems
    dt_commonroad = commonroad_data.dt; 
    dt_cpm = 0.25; % cpm lab works on 250ms time steps
    if numObst >0
        % extract trajectory of obstacles
        [obstacle_trajectory, obstacle_time]= obstacle2trajectory(commonroad_data.Obstacle_Data,dt_commonroad ,dt_cpm);
    end
    if numPlanning>0 
        % read information from planning problem
        [planning_target] = planning_extraction(commonroad_data.Planning_Data,commonroad_data.r_tree, commonroad_data.map);
        assert(numVehicles<=numPlanning);
    end
    %% initialize vehicles 
    
    StartLaneletId = 0*(1:numVehicles); % dummy zeros, because GUI initialize vehicles somewhere random
    
    % flags for Initialization of each vehicle
    VehNotStarted = ones(numVehicles,1);
    activeVehicleList = [];
    
    % flags
    bool_isFirstIter = uint8(1);
    bool_isOfflinePhase = uint8(1); 
    
    
    vehicles = cell(1,numVehicles);
    
    for k = 1:numVehicles
        vehicles{1,k} = vehicle(k,vehicle_ids{k},StartLaneletId(k),commonroad_data.map,commonroad_data.r_tree);
    end
   cfgVehicle = configVehicle(); % load configuration file
    
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

            
        % first iteration =  all Vehicles not initialized
    
        
        % read status. if active, initialize position. assuming
        % vehicle not starting at (0,0);
        if bool_isOfflinePhase == 1
            for k = 1:numVehicles
                if VehNotStarted(k) == 1
                        msg = ['sample.state_list(1,',num2str(k),'): x=',num2str(sample.state_list(1,k).pose_x),'; y= ',num2str(sample.state_list(1,k).pose_y)];
                        disp(msg);
                        msg = ['sample.state_list(1,',num2str(k),'): speed= ',num2str(sample.state_list(1,k).speed)];
                        disp(msg);
                        msg = ['sample.state_list(1,',num2str(k),'): yaw= ',num2str(sample.state_list(1,k).pose_yaw)];
                        disp(msg);

                        if ~(sample.state_list(1,k).pose_x == 0 && sample.state_list(1,k).pose_y == 0)
                            VehNotStarted(k) = 0;
                                
                            activeVehicleList = sort([activeVehicleList, k]);
                        end
                end
            end
                if sum(VehNotStarted) == 0 
                    % all vehicles online
                    bool_isOfflinePhase =0;
                else
                    % not all vehicles online, skip iteration until all vehicles are online
                    continue; 
                end 
        end
        
        % first time we reach this point is the first iteration of
        % the online phase
        if bool_isFirstIter == 1
            % store time as reference
            t_start = sample.t_now;
            % initialize obstacle trajectory with t_start as t=0
            if numObst >0
                obstacle_time_nanos = t_start+uint64(obstacle_time * 1e9);
                obstacle_time_nanos_max = obstacle_time_nanos(end);
            end
            % path planning and position update
            for k = 1:numVehicles
                vehicles{1,k}.update(sample.state_list(1,k));            
                vehicles{1,k}.resetPathPlanner(planning_target{1,k},commonroad_data.succGraph);    
            end
            % this section is going to be skipped in any further
            % iteration
            bool_isFirstIter =0;
        end
        

        
        
        % Call the programs to calculate the trajectories of all HLCs
        if (size(vehicle_ids) > 0)
            
                        
                for j = 1 : numVehicles
                    k = 1+ numVehicles-j; % j-- loop
                    if VehNotStarted(k) == 0
                        if vehicles{k}.turnOff==1
                            
                            continue; 
                        end
                        % position update and path update
                        vehicles{1,k}.update(sample.state_list(1,k));
                        vehicles{1,k}.updatePath(sample.t_now);
                        
                        
                        % get active vehicles with higher priority
                        higherPriorityindex = (activeVehicleList>k);
                        higherPriorityVehicles = activeVehicleList(higherPriorityindex);

                            
                            VehicleCollision =  hlc_trajectoryLastIter(higherPriorityVehicles,:);
                            
                            if numObst >0
                                % if we have obstacles, we have to
                                % pass obstacles and vehicles to
                                % hlc
                                if obstacle_time_nanos_max > sample.t_now
                                    % obstacle is still moving
                                    
                                    index = find(obstacle_time_nanos >= sample.t_now,1);
                                    ObstacleCollision = cell(1,numObst);

                                    for obst = 1:numObst
                                        obst_traj_points = obstacle_trajectory{1,obst}(:,index:end);
                                        
                                        % when trajectory ends,
                                        % we have to extend
                                        % trajectory to simulate
                                        % static obstacle
                                        timeStepsLeft= size(obst_traj_points,2);
                                        if timeStepsLeft < cfgVehicle.Hp
                                            obst_traj_points = [obst_traj_points, repmat(obst_traj_points(:,end),1,cfgVehicle.Hp-timeStepsLeft +1)]; 
                                        end
                                        ObstacleCollision{1,obst} =  obst_traj_points;
                                    end
                                else
                                    % obstacle has reached end of
                                    % trajectory and is not moving
                                    % anymore
                                    ObstacleCollision = cell(1,numObst);

                                    for obst = 1:numObst
                                        ObstacleCollision{1,obst} =  repmat(obstacle_trajectory{1,obst}(:,end),1,cfgVehicle.Hp+1);
                                    end
                                end
                                if isempty(VehicleCollision)
                                    dynamicObstaclesVehicles = ObstacleCollision;
                                else
                                    dynamicObstaclesVehicles = [VehicleCollision,ObstacleCollision]; % cell arrays
                                end
                            else
                                % no obstacles, we only have to
                                % pass vehicle trajectory to hlc
                                dynamicObstaclesVehicles = VehicleCollision;
                            end
                        
                        
                        %% call MPC
                        vehicles{k}.optimizeTraj(dynamicObstaclesVehicles);
                        [msg, position, time ]= vehicles{1,k}.getTrajMsg();
                        vehicle_command_trajectory_writer(msg, matlabDomainId);
                        
                        % write hlc output into storage to pass it
                        % to lower priorized vehicles
                        hlc_trajectoryLastIter{k,1} = position;  % update;
                        hlc_trajectoryLastIter{k,2} = time;  % update;
                        if vehicles{k}.turnOff==0
                            % vehicle has not reached goal
                        else
                            % vehicle reached goal
                            disp(['\n vehicle ', num2str(vehicles{k}.ext_vehicle_id),' at goal and going offline \n ']);
                        end
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
