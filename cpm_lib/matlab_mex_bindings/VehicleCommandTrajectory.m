% Contains an object definition of VehicleCommandTrajectory for Matlab, to be used for vehicle_command_trajectory_writer
classdef VehicleCommandTrajectory
    properties
        vehicle_id uint8 = 0

        create_stamp uint64 = 0
        valid_after_stamp uint64 = 0

        trajectory_points (1,:) TrajectoryPoint
    end
end