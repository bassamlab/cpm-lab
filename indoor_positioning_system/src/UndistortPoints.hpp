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
#include "cpm/dds/LedPointsPubSubTypes.h"

/**
 * \class UndistortPoints
 * \brief TODO
 * \ingroup ips
 */
class UndistortPoints
{

    //! TODO
    std::vector<double> calibration_x;
    //! TODO
    std::vector<double> calibration_y;


public:
    /**
     * \brief Constructor TODO
     * \param _calibration_x TODO
     * \param _calibration_y TODO
     */
    UndistortPoints(
        std::vector<double> _calibration_x, 
        std::vector<double> _calibration_y
    );

    /**
     * \brief Converts the vehicle LED points from image coordinates to floor coordiantes
     * \param led_points TODO
     */
    FloorPoints apply(LedPoints led_points);
    
    
};