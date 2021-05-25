% Contains an object definition of ReadyStatus for Matlab, to be used for ready_signal_writer
classdef ReadyStatus
    properties
        source_id string = ''
        next_start_stamp uint64 = 0
    end
    methods
        function obj = ReadyStatus(source_id_, next_start_stamp_)
            obj.source_id = source_id_;
            obj.next_start_stamp = next_start_stamp_;
        end
    end
end