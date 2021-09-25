% Contains an object definition of VehicleCommandPathTracking for Matlab, to be used for vehicle_command_path_tracking_writer
classdef VehicleCommandPathTracking
    properties
        vehicle_id uint8 = 0

        create_stamp uint64 = 0
        valid_after_stamp uint64 = 0

        speed double = 0.0

        path (1,:) PathPoint
    end
end