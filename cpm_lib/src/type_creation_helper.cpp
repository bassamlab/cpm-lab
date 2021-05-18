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

#include "cpm/type_creation_helper.hpp"

/**
 * \file type_creation_helper.cpp
 * \ingroup cpmlib
 */

TimeStamp cpm::create_timestamp(uint64_t nanoseconds) {
    TimeStamp time_stamp;
    time_stamp.nanoseconds(nanoseconds);
    return time_stamp;
}

TrajectoryPoint cpm::create_trajectory_point(TimeStamp t, double px, double py, double vx, double vy)
{
    TrajectoryPoint trajectory_point;
    trajectory_point.t(t);
    trajectory_point.px(px);
    trajectory_point.py(py);
    trajectory_point.vx(vx);
    trajectory_point.vy(vy);

    return trajectory_point;
}