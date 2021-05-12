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

#include "VehicleCommandPathTrackingPubSubTypes.h"

/**
 * \struct PathInterpolation
 * \brief TODO
 * \ingroup vehicle
 */
struct PathInterpolation {
    //! TODO
    double s_queried;
    //! TODO
    double position_x;
    //! TODO
    double position_y;
    //! TODO
    double velocity_x;
    //! TODO
    double velocity_y;
    //! TODO
    double acceleration_x;
    //! TODO
    double acceleration_y;
    //! TODO
    double yaw;
    //! TODO
    double speed;
    //! TODO
    double curvature;

    /**
     * \brief TODO
     * \param s_queried TOOD
     * \param start_point TOOD
     * \param end_point TOOD
     */
    PathInterpolation(const double s_queried, const PathPoint start_point, const PathPoint end_point);
};