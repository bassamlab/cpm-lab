#pragma once
#include "types.hpp"
#include "UndistortPoints.hpp"
#include "cpm/dds/LedPointsPubSubTypes.h"
#include "DetectVehicles.hpp"
#include "DetectVehicleID.hpp"
#include "PoseCalculation.hpp"
#include <memory>
#include <mutex>
#include <thread>
#include "cpm/Writer.hpp"
#include "cpm/Constants.hpp"

/**
 * \struct IpsVisualizationInput
 * \brief TODO
 * \ingroup ips
 */
struct IpsVisualizationInput
{
    //! TODO
    VehiclePoints identifiedVehicles;

    //! TODO
    std::vector<VehicleObservation> vehicleObservations;

    //! TODO
    FloorPoints floorPoints;

    //! TODO
    VehiclePoints vehiclePoints;
};

/**
 * \class IpsPipeline
 * \brief TODO
 * \ingroup ips
 */
class IpsPipeline
{
    //! A vector containing pointers to a observation writer for every vehicle
    std::vector<std::unique_ptr<cpm::Writer<VehicleObservationPubSubType>>> writers_vehicleObservation;

    //! TODO
    std::shared_ptr<UndistortPoints> undistortPointsFn;
    //! TODO
    std::shared_ptr<DetectVehicles> detectVehiclesFn;
    //! TODO
    std::shared_ptr<DetectVehicleID> detectVehicleIDfn;
    //! TODO
    std::shared_ptr<PoseCalculation> poseCalculationFn;

    //! TODO
    VehiclePointTimeseries vehiclePointTimeseries;


    // Temporary copy, for the visualization in another thread
    //! TODO
    IpsVisualizationInput ipsVisualizationInput_buffer;
    //! TODO
    std::mutex ipsVisualizationInput_buffer_mutex;
    //! TODO
    std::thread visualization_thread;

    //! TODO
    uint64_t t_previous_nanos = 0;

public:
    /**
     * \brief Constructor TODO
     * \param enable_visualization
     */
    IpsPipeline(const bool enable_visualization);

    /**
     * \brief TODO
     * \param led_points
     */
    void apply(LedPoints led_points);

    /**
     * \brief TODO
     * \param input
     */
    cv::Mat visualization(const IpsVisualizationInput &input);

    /**
     * \brief TODO
     */
    void visualization_loop();
    
    
};