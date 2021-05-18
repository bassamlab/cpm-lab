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
#include "dds/TimeStampPubSubTypes.h"
#include "dds/VehicleCommandTrajectory.h"

namespace cpm {
    /**
     * \brief Function to create a timestamp and initialize it with nanoseconds. Useful for eProsima, especially in old RTI-using-files 
     * where the old Timestamp(nanoseconds) constructor was used a lot, which is not created by eProsimas DDS Generator
     * \param nanoseconds The nanoseconds to set for the time stamp
     * \ingroup cpmlib
     */
    TimeStamp create_timestamp(uint64_t nanoseconds);

    /**
     * \brief Function to create a trajectory point and initialize it. Useful for eProsima, especially in old RTI-using-files 
     * where the constructors that initialized all variables were used a lot, which is not created by eProsimas DDS Generator
     * \param t time stamp
     * \param px x position
     * \param py y position
     * \param vx x velocity
     * \param vy y velocity
     * \ingroup cpmlib
     */
    TrajectoryPoint create_trajectory_point(TimeStamp t, double px, double py, double vx, double vy);
}