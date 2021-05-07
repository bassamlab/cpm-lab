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
#include "defaults.hpp"
#include "cpm/TimerFD.hpp"
#include "Joystick.hpp"
#include "cpm/dds/VehicleCommandDirectPubSubTypes.h"
#include "cpm/dds/VehicleCommandSpeedCurvaturePubSubTypes.h"
#include "cpm/dds/VehicleCommandTrajectoryPubSubTypes.h"
#include <functional>

/**
 * \brief Class for controlling a vehicle with a joystick
 * \ingroup lcc
 */
class VehicleManualControl
{
    shared_ptr<Joystick> joystick = nullptr;
    //! TODO
    std::shared_ptr<cpm::TimerFD> update_loop = nullptr;
    //! TODO
    uint8_t vehicle_id = 0;
    
    //! TODO
    double ref_speed = 0;
    //! TODO
    uint64_t ref_trajectory_start_time = 0;
    //! TODO
    int ref_trajectory_index = 0;

    //! TODO
    shared_ptr<cpm::Writer<VehicleCommandDirectPubSubType>> writer_vehicleCommandDirect = nullptr;
    //! TODO
    shared_ptr<cpm::Writer<VehicleCommandSpeedCurvaturePubSubType>> writer_vehicleCommandSpeedCurvature = nullptr;

    //! TODO
    std::function<void()> m_update_callback;

public:
    /**
     * \brief TODO Constructor
     */
    VehicleManualControl();

    /**
     * \brief TODO
     * \param vehicleId TODO
     * \param joystick_device_file TODO
     */
    void start(uint8_t vehicleId, string joystick_device_file);
    
    /**
     * \brief TODO
     */
    void stop();

    /**
     * \brief TODO
     * \param update_callback TODO
     */
    void set_callback(std::function<void()> update_callback) { m_update_callback = update_callback; }

    /**
     * \brief TODO
     * \param throttle TODO
     * \param steering TODO
     */
    void get_state(double& throttle, double& steering);
};