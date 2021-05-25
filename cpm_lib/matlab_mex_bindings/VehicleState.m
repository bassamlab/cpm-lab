% Contains an object definition of VehicleState for Matlab, to be used for ready_signal_writer
classdef VehicleState
    properties
        vehicle_id uint8 = 0

        pose Pose2D

        IPS_update_age_nanoseconds uint64 = 0
        odometer_distance double = 0
        imu_acceleration_forward double = 0
        imu_acceleration_left double = 0
        imu_acceleration_up double = 0
        imu_yaw double = 0
        imu_yaw_rate double = 0
        speed double = 0
        battery_voltage double = 0
        motor_current double = 0

        motor_throttle double = 0
        steering_servo double = 0

        is_real logical = false
    end
end