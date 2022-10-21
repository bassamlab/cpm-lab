#pragma once

#include "cpm/dds/VehicleCommandDirectPubSubTypes.h"
#include "cpm/dds/VehicleCommandSpeedCurvaturePubSubTypes.h"
#include "cpm/dds/VehicleCommandTrajectoryPubSubTypes.h"
#include "cpm/dds/VehicleCommandPathTrackingPubSubTypes.h"
#include "cpm/dds/VehicleStateListPubSubTypes.h"
#include <map>
#include <memory>
#include <mutex>
#include "cpm/VehicleIDFilteredTopic.hpp"
#include "cpm/ParticipantSingleton.hpp"
#include "cpm/Reader.hpp"
#include "cpm/AsyncReader.hpp"

#include "MpcController.hpp"
#include "PathTrackingController.hpp"
#include "TrajectoryInterpolation.hpp"

extern "C" {
    #include "../../low_level_controller/vehicle_atmega2560_firmware/spi_packets.h"
}

#define TRAJECTORY_TRACKING_STATISTICS_BUFFER_SIZE 1500

/**
 * \enum ControllerState
 * \brief TODO
 * \ingroup vehicle
 */
enum class ControllerState
{
    Stop,
    Direct,
    SpeedCurvature,
    Trajectory,
    PathTracking
};

/**
 * \class Controller
 * \brief TODO
 * \ingroup vehicle
 */
class Controller
{
    //! TODO
    MpcController mpcController;
    //! TODO
    PathTrackingController pathTrackingController;

    //! TODO
    std::function<uint64_t()> m_get_time;

    //cpm::VehicleIDFilteredTopic<VehicleCommandDirect> topic_vehicleCommandDirect;
    //! TODO
    std::unique_ptr< cpm::Reader<VehicleCommandDirectPubSubType> > reader_CommandDirect;

    //cpm::VehicleIDFilteredTopic<VehicleCommandSpeedCurvature> topic_vehicleCommandSpeedCurvature;
    //! TODO
    std::unique_ptr< cpm::Reader<VehicleCommandSpeedCurvaturePubSubType> > reader_CommandSpeedCurvature;

    //cpm::VehicleIDFilteredTopic<VehicleCommandTrajectory> topic_vehicleCommandTrajectory;
    //! TODO
    std::unique_ptr< cpm::Reader<VehicleCommandTrajectoryPubSubType> > reader_CommandTrajectory;

    //cpm::VehicleIDFilteredTopic<VehicleCommandPathTracking> topic_vehicleCommandPathTracking;
    //! TODO
    std::unique_ptr< cpm::Reader<VehicleCommandPathTrackingPubSubType>> reader_CommandPathTracking;

    //! TODO
    VehicleState m_vehicleState;

    //! Last received VehicleCommandDirect message
    VehicleCommandDirect m_vehicleCommandDirect;
    //! Buffer VehicleCommandDirect sample age (when read with get_newest_sample(): (t_now - sample_out.header().valid_after_stamp().nanoseconds()))
    uint64_t m_sample_CommandDirect_age;

    //! Last received VehicleSpeedCurvature message
    VehicleCommandSpeedCurvature m_vehicleCommandSpeedCurvature;
    //! Buffer VehicleSpeedCurvature sample age (when read with get_newest_sample(): (t_now - sample_out.header().valid_after_stamp().nanoseconds()))
    uint64_t m_sample_CommandSpeedCurvature_age;

    //! Last received VehicleCommandTrajectory message
    VehicleCommandTrajectory m_vehicleCommandTrajectory;
    //! Buffer VehicleCommandTrajectory sample age (when read with get_newest_sample(): (t_now - sample_out.header().valid_after_stamp().nanoseconds()))
    uint64_t m_sample_CommandTrajectory_age;

    //! Last received VehicleCommandPathTracking message
    VehicleCommandPathTracking m_vehicleCommandPathTracking;
    //! Buffer VehicleCommandPathTracking sample age (when read with get_newest_sample(): (t_now - sample_out.header().valid_after_stamp().nanoseconds()))
    uint64_t m_sample_CommandPathTracking_age;
    
    //! Buffer for trajectory interpolation
    TrajectoryPoint b_start_point = TrajectoryPoint();
    //! Buffer for trajectory interpolation
    TrajectoryPoint b_end_point = TrajectoryPoint();

    //! TODO
    uint8_t vehicle_id;

    //! TODO
    ControllerState state = ControllerState::Stop;

    //! TODO
    const uint64_t command_timeout = 1000000000ull;

    //! TODO
    double speed_throttle_error_integral = 0;

    //! TODO
    std::mutex command_receive_mutex;

    // TODO remove linear trajectory controller related stuff, once the MPC works well
    //! TODO
    double trajectory_controller_lateral_P_gain;
    //! TODO
    double trajectory_controller_lateral_D_gain;
    /**
     * \brief TODO
     * \param t_now TODO
     * \param motor_throttle_out TODO
     * \param steering_servo_out TODO
     */
    void trajectory_controller_linear(uint64_t t_now, double &motor_throttle_out, double &steering_servo_out);

    /**
     * \brief TODO
     */
    void update_remote_parameters();

    /**
     * \brief TODO
     * \param speed_measured TODO
     * \param speed_target TODO
     */
    double speed_controller(const double speed_measured, const double speed_target);

    /**
     * \brief TODO
     * \param t_now TODO
     */
    void receive_commands(uint64_t t_now);

    /**
     * \brief TODO
     * \param t_now TODO
     */
    std::shared_ptr<TrajectoryInterpolation> interpolate_trajectory_command(uint64_t t_now);

    // Trajectory tacking statistics
    /**
     * \brief TODO
     * \param t_now TODO
     */
    void trajectory_tracking_statistics_update(uint64_t t_now);
    //! TODO
    double trajectory_tracking_statistics_longitudinal_errors  [TRAJECTORY_TRACKING_STATISTICS_BUFFER_SIZE];
    //! TODO
    double trajectory_tracking_statistics_lateral_errors       [TRAJECTORY_TRACKING_STATISTICS_BUFFER_SIZE];
    //! TODO
    size_t trajectory_tracking_statistics_index = 0;

public:
    /**
     * \brief TODO
     * \param vehicle_id TODO
     * \param _get_time TODO
     */
    Controller(uint8_t vehicle_id, std::function<uint64_t()> _get_time);

    /**
     * \brief TODO
     * \param vehicleState TODO
     */
    void update_vehicle_state(VehicleState vehicleState);

    /**
     * \brief TODO
     * \param stamp_now TODO
     * \param motor_throttle TODO
     * \param steering_servo TODO
     */
    void get_control_signals(uint64_t stamp_now, double &motor_throttle, double &steering_servo);

    /**
     * \brief Uses a P controller based on speed_controller (which uses a PI controller) to calculate the correct values for stopping the car (only for that)
     * \param motor_throttle Motor setting required to stop
     * \param steering_servo Steering value
     */
    void get_stop_signals(double &motor_throttle, double &steering_servo);

    /**
     * \brief Resets Reader and trajectory list, sets the current state to stop
     */
    void reset();
};
