% Contains an object definition of VehicleStateList for Matlab, to be used for ready_signal_writer
classdef VehicleStateList
    properties
        t_now uint64 = 0
        period_ms uint64 = 0

        state_list (1,:) VehicleState = [VehicleState] % We have to set one object, to be able to create more in the MEX file
        vehicle_observation_list (1,:) VehicleObservation = [VehicleObservation] % We have to set one object, to be able to create more in the MEX file
        active_vehicle_ids (1,:) int32 = [int32.empty]

        is_valid logical = false % IMPORTANT: The reader might not receive anything, in this case is_valid is set to false
    end
end