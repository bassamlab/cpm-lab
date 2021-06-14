% Contains an object definition of ReadyStatus for Matlab, to be used for ready_signal_writer
classdef ReadyStatus
    properties
        source_id string = ''
        next_start_stamp uint64 = 0
    end
end