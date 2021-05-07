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
#include "casadi_mpc_fn.h"
#include <map>
#include <string>
#include <array>
#include <vector>
#include "VehicleModel.hpp"
#include "cpm/dds/VehicleCommandTrajectoryPubSubTypes.h"
#include "cpm/dds/VehicleStateListPubSubTypes.h"
#include "cpm/dds/VisualizationPubSubTypes.h"
#include "cpm/get_topic.hpp"
#include "cpm/Writer.hpp"



/* The MPC prediction and the gradient descent 
 * optimization are developed in Matlab with CasADi.
 * Here we just call the resulting generated code.
 * See tools/vehicle_dynamics_identification_and_mpc/MpcController.m
 */

#define MPC_DELAY_COMPENSATION_STEPS (3)

/**
 * \class MpcController
 * \brief TODO
 * \ingroup vehicle
 */
class MpcController
{
    //! TODO
    cpm::Writer<VisualizationPubSubType> writer_Visualization;
    //! TODO 
    uint8_t vehicle_id;

    //! TODO
    std::map< std::string, std::vector<casadi_real> > casadi_vars;
    //! TODO
    std::map< std::string, std::array<casadi_int, 2> > casadi_vars_size;

    //! TODO
    std::vector<casadi_real*> casadi_arguments;
    //! TODO
    std::vector<casadi_real*> casadi_results;

    //! TODO
    const size_t MPC_prediction_steps = 6;
    //! TODO
    const size_t MPC_control_steps = 3;
    //! The period in which update() is called
    const double dt_control_loop = 0.02; 
    //! The MPC prediction time step
    const double dt_MPC = 0.05;


    //! TODO load parameters via DDS parameters
    std::vector<double> dynamics_parameters = { 1.004582, -0.142938, 0.195236, 3.560576, -2.190728, -9.726828, 2.515565, 1.321199, 0.032208, -0.012863 };

    //! TODO
    double battery_voltage_lowpass_filtered = 8;


    //! Holds the N most recent outputs/commands. oldest first, newest last
    double motor_output_history[MPC_DELAY_COMPENSATION_STEPS];
    //! TODO
    double steering_output_history[MPC_DELAY_COMPENSATION_STEPS];


    // Take the current state measurement and predict it into the future a few steps.
    // This is necessary to compensate the delay of the inputs.
    /**
     * \brief TODO
     * \param vehicleState TODO
     */
    VehicleState delay_compensation_prediction(
        const VehicleState &vehicleState
    );


    // Interpolates the reference trajectory on the MPC time grid.
    // Returns false if the reference trajectory is not defined for the 
    // MPC prediction time interval, or is impossible to follow.
    /**
     * \brief TODO
     * \param t_now TODO
     * \param commandTrajectory TODO
     * \param out_mpc_reference_trajectory_x TODO
     * \param out_mpc_reference_trajectory_y TODO
     */
    bool interpolate_reference_trajectory(
        uint64_t t_now, 
        const VehicleCommandTrajectory &commandTrajectory,
        std::vector<double> &out_mpc_reference_trajectory_x,
        std::vector<double> &out_mpc_reference_trajectory_y
    );


    /**
     * \brief TODO
     * \param vehicleState_predicted_start TODO
     * \param mpc_reference_trajectory_x TODO
     * \param mpc_reference_trajectory_y TODO
     * \param out_motor_throttle TODO
     * \param out_steering_servo TODO
     */
    void optimize_control_inputs(
        const VehicleState &vehicleState_predicted_start,
        const std::vector<double> &mpc_reference_trajectory_x,
        const std::vector<double> &mpc_reference_trajectory_y,
        double &out_motor_throttle, 
        double &out_steering_servo
    );

    /**
     * \brief TODO
     */
    void reset_optimizer();

    //! TODO
    std::function<void(double&, double&)> stop_vehicle;


public:
    /**
     * \brief TODO
     * \param _vehicle_id TODO
     * \param _stop_vehicle TODO
     */
    MpcController(uint8_t _vehicle_id, std::function<void(double&, double&)> _stop_vehicle);

    /**
     * \brief TODO
     * \param t_now TODO
     * \param vehicleState TODO
     * \param commandTrajectory TODO
     * \param out_motor_throttle TODO
     * \param out_steering_servo TODO
     */
    void update(
        uint64_t t_now, 
        const VehicleState &vehicleState,                  
        const VehicleCommandTrajectory &commandTrajectory,
        double &out_motor_throttle, 
        double &out_steering_servo
    );
    
};