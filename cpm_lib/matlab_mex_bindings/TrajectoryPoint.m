% Contains an object definition of TrajectoryPoint for Matlab, to be used for ready_signal_writer
classdef TrajectoryPoint
    properties
        t uint64 = 0

        px double = 0
        py double = 0
        vx double = 0
        vy double = 0
    end
end