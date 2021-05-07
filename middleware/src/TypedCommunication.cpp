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

#include "TypedCommunication.hpp"

/**
 * \file TypedCommunication.cpp
 * \ingroup middleware
 */

template<> void TypedCommunication<VehicleCommandTrajectoryPubSubType>::type_specific_msg_check(VehicleCommandTrajectory& msg)
{
    auto set_id = msg.vehicle_id();

    //1. Make sure that enough points have been set (2 points or less are not sufficient)
    if (msg.trajectory_points().size() < 3)
    {
        cpm::Logging::Instance().write(
            1,
            "Middleware (ID %i): HLC script sent too few trajectory points, cannot be used for interpolation",
            static_cast<int>(set_id)
        );
    }

    //2. Check how many of the set trajectory points lie in the past / future
    size_t num_past_trajectories = 0;
    auto current_time = cpm::get_time_ns();
    for (auto point : msg.trajectory_points())
    {
        if (point.t().nanoseconds() < current_time)
        {
            ++num_past_trajectories;
        }
        else
        {
            break;
        }
        
    }
    //  a) At least one trajectory point must be in the past, or interpolation is not possible
    if (num_past_trajectories == 0)
    {
        cpm::Logging::Instance().write(
            1,
            "Middleware (ID %i): HLC script sent no past trajectory points, cannot be used for interpolation",
            static_cast<int>(set_id)
        );
    }
    //  b) At least one trajectory point must be in the future, or interpolation is not possible
    if (num_past_trajectories == msg.trajectory_points().size())
    {
        cpm::Logging::Instance().write(
            1,
            "Middleware (ID %i): HLC script sent no future trajectory points, cannot be used for interpolation",
            static_cast<int>(set_id)
        );
    }
}

template<> void TypedCommunication<VehicleCommandPathTracking>::type_specific_msg_check(VehicleCommandPathTracking msg)
{
    auto set_id = msg.vehicle_id();
    auto path_length = msg.path().size();

    //1. Make sure that enough points have been set (less than 2 points are not sufficient)
    if (path_length < 2) {
        cpm::Logging::Instance().write(
            1,
            "Middleware (ID %i): HLC script sent sent too few path tracking points, cannot be used for interpolation",
            static_cast<int>(set_id)
        );
    }

    //2. Make sure the first path tracking point is valid (s must be zero)
    if (msg.path().at(0).s() != 0) {
        cpm::Logging::Instance().write(
            1,
            "Middleware (ID %i): HLC script sent invalid first path point, s must be 0",
            static_cast<int>(set_id)
        );
    }

    //3. Make sure consecutive path points have increasing s
    for (std::size_t i = 0; i < path_length - 1; ++i)
    {
        std::size_t j = i + 1;
        if (msg.path().at(j).s() <= msg.path().at(i).s()) {
            cpm::Logging::Instance().write(
                1,
                "Middleware (ID %i): HLC script sent invalid path points, s must be increasing",
                static_cast<int>(set_id)
            );
        }
    }

    //4. Make sure first and last pose are identical
    Pose2D first = msg.path().at(0).pose();
    Pose2D last = msg.path().at(path_length - 1).pose();

    if (first.x() != last.x())
    {
        cpm::Logging::Instance().write(
            1,
            "Middleware (ID %i): HLC script sent invalid path points, first and last x value differ",
            static_cast<int>(set_id)
        );
    }

    if (first.y() != last.y())
    {
        cpm::Logging::Instance().write(
            1,
            "Middleware (ID %i): HLC script sent invalid path points, first and last y value differ",
            static_cast<int>(set_id)
        );
    }

    if (first.yaw() != last.yaw())
    {
        cpm::Logging::Instance().write(
            1,
            "Middleware (ID %i): HLC script sent invalid path points, first and last yaw value differ",
            static_cast<int>(set_id)
        );
    }
}