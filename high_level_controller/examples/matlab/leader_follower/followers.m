function [msg, previous_leader_states]=followers(vehicle_id, sample, follow_id, t_now, previous_leader_states, max_previous_states)
    %% Do not display figures
    set(0,'DefaultFigureVisible','off');

    state_list = sample.state_list;

    % Get time (same as in trajectory_complex)
    point_period_nanoseconds = 250000000;
    t_eval = ((t_now + uint64(500000000)) / point_period_nanoseconds) * point_period_nanoseconds;

    % Create msg
    trajectory = VehicleCommandTrajectory;
    trajectory.vehicle_id = uint8(vehicle_id);
    trajectory_points = [];

    % Search for vehicle position (follow_id) in state list
    for i = 1:length(state_list)
        if state_list(i).vehicle_id == follow_id
            % Update list of 3 previous leader states with the current entry
            % Different behaviour in case the list does not yet contain 3 entries
            if length(previous_leader_states{follow_id}) < max_previous_states
                % Add new entry
                previous_leader_states{follow_id} = [previous_leader_states{follow_id} state_list(i)];
            else
                % Get rid of oldest entry, keep order (oldest to newest)
                new_states = circshift(previous_leader_states{follow_id}, -1);
                new_states(max_previous_states) = state_list(i);
                previous_leader_states{follow_id} = new_states;
            end

            break;
        end
    end

    leader_states = previous_leader_states{follow_id};
    for i = 1 : length(leader_states)
        follow_state = leader_states(i);

        point1 = TrajectoryPoint;

        % Use sample time of creation, as vehicle state list data is obtained with that frequency
        first_state = leader_states(1);
        time = t_eval + 400000000 + (follow_state.create_stamp - first_state.create_stamp);
        
        point1.t = uint64(time);
        point1.px = follow_state.pose_x;
        point1.py = follow_state.pose_y;
        point1.vx = cos(follow_state.pose_yaw) * follow_state.speed;
        point1.vy = sin(follow_state.pose_yaw) * follow_state.speed;

        trajectory_points = [trajectory_points [point1]];
    end
    trajectory.trajectory_points = trajectory_points;
    
    trajectory.create_stamp = uint64(t_now);
    trajectory.valid_after_stamp = uint64(t_now + 200e6);

    % Return msg
    msg = trajectory;
end