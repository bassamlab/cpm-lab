#pragma once
#include <vector>
#include "cpm/dds/VehicleCommandPathTrackingPubSubTypes.h"
#include "cpm/dds/VehicleStatePubSubTypes.h"
#include "cpm/dds/VisualizationPubSubTypes.h"
#include "cpm/Writer.hpp"


/**
 * \class PathTrackingController
 * \brief TODO
 * \ingroup vehicle
 */
class PathTrackingController
{
    //! TOOD
    cpm::Writer<VisualizationPubSubType> writer_Visualization;
    //! TOOD
    uint8_t vehicle_id;

    /**
     * \brief TODO
     * \param path TODO
     * \param x TODO
     * \param y TODO
     */
    Pose2D find_reference_pose(
        const std::vector<PathPoint> &path,
        const double x,
        const double y
    );

public:
    /**
     * \brief TODO Constructor
     * \param vehicle_id TODO
     */
    PathTrackingController(uint8_t vehicle_id);

    /**
     * \brief TODO
     * \param vehicleState TODO
     * \param commandPathTracking TODO
     */
    double control_steering_servo(
        const VehicleState &vehicleState,
        const VehicleCommandPathTracking &commandPathTracking
    );
};