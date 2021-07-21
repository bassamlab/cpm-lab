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

function main_vehicle_matlab(varargin)

%% 
% Diese Version wird zum Debugging genutzt und kann von Matlab aus gestartet werden mit main_vehicle_matlab(1,2,..);
% 
%%

    % Get current path
    clc
    script_directoy = fileparts([mfilename('fullpath') '.m']);
    cd(script_directory);

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
    %% IBM CPLEX path
    CPLEX_PATH = '/opt/ibm/ILOG/CPLEX_Studio_Community129/cplex/matlab/x86-64_linux';
    
    addpath(CPLEX_PATH);
    
    
    %% Lanelet based motion
    addpath('./RTree');
    addpath('./ControllerFiles');
    addpath('./Maps');
    addpath('./CommonRoadImport');
%              filepath = './Maps/LabMapCommonRoadPlanningObstacle.xml';
  % filepath = './Maps/scenarios_cooperative_C-USA_Lanker-2_2_T-1.xml';
%    filepath='./Maps/ZAM_Tjunction-1_144_T-1.xml';
filepath = './Maps/LabMapCommonRoadPlanning2Vehicles.xml';
    
    commonroad_data = LoadXML(filepath); 
    numVehicles = length(vehicle_ids);
    numObst = commonroad_data.Obstacle_Data.numObst;
    numPlanning = commonroad_data.Planning_Data.numPlanning;
    dt_commonroad = commonroad_data.dt;
    
    dt_cpm = 0.25; 
    if numObst >0
        [obstacle_trajectory, obstacle_time]= obstacle2trajectory(commonroad_data.Obstacle_Data,dt_commonroad ,dt_cpm);
    end
    
    if numPlanning>0
        [planning_target] = planning_extraction(commonroad_data.Planning_Data,commonroad_data.r_tree, commonroad_data.map);
        assert(numVehicles<=numPlanning);
    end
    
    %% enable drawing
%     set(0,'DefaultFigureVisible','on');
%     figure
%       drawCenterline(commonroad_data.r_tree,commonroad_data.map)
    
    % not working yet. should initialize vehicles to their starting
    % position
    StartLaneletId = 0*(1:numVehicles);
    % flags for Initialization of each vehicle, allowing asynchrous start
    VehNotStarted = ones(numVehicles,1);
    activeVehicleList = [];
    
    vehicles = cell(numVehicles);
    hlc_trajectoryNew = cell(numVehicles,2);
    hlc_trajectoryLastIter = cell(numVehicles,2);
    for k = 1:numVehicles
        vehicles{k} = vehicle(k,vehicle_ids{k},StartLaneletId(k),commonroad_data.map,commonroad_data.r_tree);
    end
   cfgVehicle = configVehicle();


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
       
    
    % Wait for start signal
    disp('Waiting for start or stop signal');
    got_stop = false;
    got_start = false;
    
    counter = 0;
    bool_isFirstIter = uint8(1);
    bool_isOfflinePhase = uint8(1); % at the begin, until all vehicles all online
 %% enter loop

    
    if got_stop == false
        while(true)
            counter=counter+1;
            
            stateSampleCount = 1;
            if counter == 1
                sample.state_list(1,1).pose.x = 3.1581 - 0.01*(counter-1);
                sample.state_list(1,1).pose.y = 3.8894+0.05*(counter-1);
                sample.state_list(1,1).pose.yaw = 1.4473;
                sample.state_list(1,1).speed = 0;
            else
                lasPos = hlc_trajectoryLastIter{k,1};
                sample.state_list(1,1).pose.x= lasPos(1,3);
                sample.state_list(1,1).pose.y= lasPos(2,3);
            end
            
            
            sample.t_now = uint64(counter * 2.5* 1e8);
            
            % Check if any new message was received (TODO: throw away all messages that were received during computation)
           if stateSampleCount > 0                
                disp('Current time:');
                disp(sample.t_now);
                
                % first iteration =  all Vehicles not initialized
            
                
                % read status. if active, initialize position. assuming
                % vehicle not starting at (0,0);
                if bool_isOfflinePhase == 1
                    for k = 1:numVehicles
                        if VehNotStarted(k) == 1
                                msg = ['sample.state_list(1,',num2str(k),'): x=',num2str(sample.state_list(1,k).pose.x),'; y= ',num2str(sample.state_list(1,k).pose.y)];
                                disp(msg);
                                msg = ['sample.state_list(1,',num2str(k),'): speed= ',num2str(sample.state_list(1,k).speed)];
                                disp(msg);
                                msg = ['sample.state_list(1,',num2str(k),'): yaw= ',num2str(sample.state_list(1,k).pose.yaw)];
                                disp(msg);

                                if ~(sample.state_list(1,k).pose.x == 0 && sample.state_list(1,k).pose.y == 0)
                                    VehNotStarted(k) = 0;
                                     
                                    activeVehicleList = sort([activeVehicleList, k]);
                                end
                        end
                    end
                        if sum(VehNotStarted) == 0
                            bool_isOfflinePhase =0;
                        else
                            continue;
                        end 
                end
                if bool_isFirstIter == 1
                   t_start = sample.t_now;
                   if numObst >0
                       obstacle_time_nanos = t_start+uint64(obstacle_time * 1e9);
                        obstacle_time_nanos_max = obstacle_time_nanos(end);
                   end
                   for k = 1:numVehicles
                        vehicles{1,k}.update(sample.state_list(1,k));            
                        vehicles{1,k}.resetPathPlanner(planning_target{1,k},commonroad_data.succGraph);    
                   end
                   bool_isFirstIter =0;
                end
                
                % Call the programs to calculate the trajectories of all HLCs
                if (size(vehicle_ids) > 0)
                    
                                
                        for k = 1 : numVehicles
                            if VehNotStarted(k) == 0
                                if vehicles{k}.turnOff==1
                                   continue; 
                                end
                                
                                % position update and ref traj update
                                vehicles{k}.update(sample.state_list(1,k));
                                vehicles{k}.updatePath(sample.t_now);
                                
                                
                                % get active vehicles with higher priority which
                                higherPriorityindex = (activeVehicleList>k);
%                                 numHigherPriorities = sum(higherPriorityindex);
                                higherPriorityVehicles = activeVehicleList(higherPriorityindex);

                                    
                                    VehicleCollision =  hlc_trajectoryLastIter(higherPriorityVehicles,1);
                                    
                                    if numObst >0
                                        if obstacle_time_nanos_max > sample.t_now
                                            % get obstacle trajectory
                                            index = find(obstacle_time_nanos >= sample.t_now,1);
                                            ObstacleCollision = cell(1,numObst);

                                            for obst = 1:numObst
                                                obst_traj_points = obstacle_trajectory{1,obst}(:,index:end);
                                                
                                                % when trajectory hits end,
                                                % we have to extend
                                                % trajectory to simulate
                                                % static obstacle
                                                timeStepsGiven = size(obst_traj_points,2);
                                                if timeStepsGiven < cfgVehicle.Hp
                                                   obst_traj_points = [obst_traj_points, repmat(obst_traj_points(:,end),1,cfgVehicle.Hp-timeStepsGiven +1)]; 
                                                end
                                                ObstacleCollision{1,obst} =  obst_traj_points;
                                            end
                                        else
                                            ObstacleCollision = cell(1,numObst);

                                            for obst = 1:numObst
                                                ObstacleCollision{1,obst} =  repmat(obstacle_trajectory{1,obst}(:,end),1,cfgVehicle.Hp+1);
                                            end
                                        end
                                        %TODO: check if they are empty when merging
                                        dynamicObstaclesVehicles = [VehicleCollision;ObstacleCollision];
                                    else
                                        dynamicObstaclesVehicles = VehicleCollision;
                                    end
                                
                                  if counter >33
                                     1;
                                  end
                                 
                                  
                                vehicles{k}.optimizeTraj(dynamicObstaclesVehicles);
                                [msg, position, time ]= vehicles{k}.getTrajMsg();
                                if vehicles{k}.turnOff==0
                                    vehicle_command_trajectory_writer(msg);
                                    hlc_trajectoryLastIter{k,1} = position;  % update;
                                    hlc_trajectoryLastIter{k,2} = time;  % update;
                                else
                                    % do nothing, vehicle is deactivated
                                end
                            end
                        end
                    
                end
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
