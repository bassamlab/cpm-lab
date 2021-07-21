% Contains an object definition of VehicleCommandDirect for Matlab
classdef VehicleCommandDirect
    properties
        vehicle_id uint8 = 0

        create_stamp uint64 = 0
        valid_after_stamp uint64 = 0

        motor_throttle double = 0.0
        steering_servo double = 0.0
    end
end