% Contains an object definition of SystemTrigger for Matlab, to be used for ready_signal_writer
classdef SystemTrigger
    properties
        next_start uint64 = 0
        is_valid logical = false % IMPORTANT: The reader might not receive anything, in this case is_valid is set to false
    end
end