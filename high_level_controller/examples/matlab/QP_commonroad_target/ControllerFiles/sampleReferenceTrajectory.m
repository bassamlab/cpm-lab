
% Computes reference trajectory points for a piecewise linear curve. Makes uses of the
% reference speed on path points.

function [ RefData ] = sampleReferenceTrajectory(Hp, path_pos, path_speed,currPos,dt)

% just to be sure that there is enough trajectory points available for collision handling
    n_path =   Hp+5; % number of path points
    
    ref_length = length(path_speed)+1;
    if ref_length >= n_path
        % referenceTrajectory is non-equidistant array of centerline [[x1;y1],...,[xn;yn]]
        % where [x1;y1] is the next sampling point in driving direction
        PosPath = [currPos,path_pos];
        ref_speed = [path_speed(1) , path_speed]; % ref speed local coordinate system
    else
        % case where planned path has less control points than n_path, e.g.
        % if close to goal 
        ref_speed = zeros(1,n_path);
        PosPath = zeros(2,n_path);
        
        ref_speed(:,1:ref_length)  = [path_speed(1) , path_speed]; % ref speed local coordinate system
        PosPath(:,1:ref_length) = [currPos,path_pos];
       
        ref_speed(ref_length+1:n_path) = 0*(ref_length+1:n_path) ;
        PosPath(:, ref_length+1:n_path)= path_pos(:, end) + 0 * PosPath(:, ref_length+1:n_path);
    end
    
    
    DeltaP = diff( PosPath,1,2);
    DeltaS = [sqrt(DeltaP(1,:).^2 + DeltaP(2,:).^2)];
    s_coordinate = [0, cumsum(DeltaS)]; % local coordinate system
    maxDist = s_coordinate(end); % maximum allowed distance in horizon
    
    
    samplingPos = zeros(1,n_path); % reference trajectory : position
    samplingSpeed = zeros(1,n_path);
    
    samplingPos(1) = 0 + path_speed(1) * dt;
    thresholdIndex = find( s_coordinate > samplingPos(1));
    
    % the curr point has to be between SpeedIndex and SpeedIndex +1. 
    
    % for k=1
    SpeedIndex = min (max (thresholdIndex(1)-1,1), n_path-1 ); 
    
    % A)choose minimum speed to be below speedlimits
%     samplingSpeed(1) = min( ref_speed(SpeedIndex),ref_speed(SpeedIndex+1)) ;
    
    % B) or choose first entry to reach goal
    samplingSpeed(1) = ref_speed(SpeedIndex);
    
    
   
    for k=2:n_path
            samplingPos(k) = samplingPos(k-1) + samplingSpeed(k-1) *dt;
            SpeedIndex = find( s_coordinate > samplingPos(k));
            if ~isempty(SpeedIndex)
                samplingSpeed(k) = min( ref_speed(SpeedIndex(1)),ref_speed(min(SpeedIndex(1)+1,ref_length))) ; 
            else
               % happens at the end , we have to throttle last step
               
               
               samplingPos(k:end) = maxDist + 0 * samplingPos(k:end) ;
               samplingSpeed(k+1:end) = 0*samplingSpeed(k+1:end);
               samplingSpeed(k) = (maxDist  - samplingPos(k-1))/dt; 
               
               break;           
               
            end
    end
    
    % Interpolation setup
    pos_interpolation = [currPos,path_pos];
    speed_interpolation = [path_speed(1) , path_speed];
    deltaP_interpolation = diff( [currPos,path_pos] ,1,2);
    deltaS_interpolation = [sqrt(deltaP_interpolation(1,:).^2 + deltaP_interpolation(2,:).^2)];
    s_interpolation = [0, cumsum(deltaS_interpolation)]; % local coordinate system
    
    
    % local coordinate for mpc
    Ref_pos_mpc = samplingPos(1:Hp); % pos
    Ref_pos_mpc = min(Ref_pos_mpc,maxDist) ;% we want to stop at maxDist

    Ref_speed_mpc =  interp1(s_interpolation,speed_interpolation,Ref_pos_mpc); % desired speed, y2  
    % local coordinate for collision handling
    Ref_Points_ch = samplingPos(1:n_path);
    Ref_Points_ch = min(Ref_Points_ch,maxDist); % we want to stop at maxDist
    
    Ref_x_global = interp1(s_interpolation,pos_interpolation(1,:),Ref_Points_ch);
    Ref_y_global = interp1(s_interpolation,pos_interpolation(2,:),Ref_Points_ch);
    
    Ref_points_global = [Ref_x_global;Ref_y_global]; % global coordinate
    
    
    RefData.TrajPointsGlobal =Ref_points_global ; % global coordinate , reference for collision handling
    
    RefData.ReferenceTrajectoryPoints = [Ref_pos_mpc;Ref_speed_mpc]; % Reference trajectory for mpc, local coordinate
    
end

