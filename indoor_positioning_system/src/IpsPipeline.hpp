// MIT License
// 
// Copyright (c) 2020 Lehrstuhl Informatik 11 - RWTH Aachen University
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// This file is part of cpm_lab.
// 
// Author: i11 - Embedded Software, RWTH Aachen University

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
    //! TODO 
    cpm::Writer<VehicleObservationPubSubType> writer_vehicleObservation;

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