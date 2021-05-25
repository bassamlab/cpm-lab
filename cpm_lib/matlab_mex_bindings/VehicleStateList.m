% Contains an object definition of VehicleStateList for Matlab, to be used for ready_signal_writer
classdef VehicleStateList
    properties
        t_now uint64 = 0
        period_ms uint64 = 0

        state_list VehicleState = VehicleState.empty
        vehicle_observation_list VehicleObservation = VehicleObservation.empty
        active_vehicle_ids int32 = int32.empty

        is_valid logical = true % IMPORTANT: The reader might not receive anything, in this case is_valid is set to false
    end
end