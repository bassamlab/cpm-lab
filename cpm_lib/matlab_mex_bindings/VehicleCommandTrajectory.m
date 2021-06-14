% Contains an object definition of VehicleCommandTrajectory for Matlab, to be used for ready_signal_writer
classdef VehicleCommandTrajectory
    properties
        vehicle_id uint8_t = 0

        create_stamp uint64_t = 0
        valid_after_stamp uint64_t = 0

        trajectory_points (1,:) TrajectoryPoint = []
    end
end