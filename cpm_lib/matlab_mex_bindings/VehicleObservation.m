% Contains an object definition of VehicleObservation for Matlab, to be used for ready_signal_writer
classdef VehicleObservation
    properties
        vehicle_id uint8 = 0

        pose Pose2D
    end
end