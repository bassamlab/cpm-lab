% Contains an object definition of VehicleCommandSpeedCurvature for Matlab
classdef VehicleCommandSpeedCurvature
    properties
        vehicle_id uint8 = 0

        create_stamp uint64 = 0
        valid_after_stamp uint64 = 0

        speed double = 0.0
        curvature double = 0.0
    end
end