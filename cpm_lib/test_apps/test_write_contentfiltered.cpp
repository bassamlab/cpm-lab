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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
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

#include "cpm/Logging.hpp"
#include "cpm/ParticipantSingleton.hpp"
#include "cpm/dds/VehicleStatePubSubTypes.h"
#include "cpm/dds/VehicleObservationPubSubTypes.h"
#include "cpm/dds/VehicleCommandDirectPubSubTypes.h"
#include "cpm/dds/ColorPubSubTypes.h"

#include "cpm/Reader.hpp"
#include "cpm/Writer.hpp"
#include "cpm/InternalConfiguration.hpp"
#include <unistd.h>

int main(int argc, char *argv[]){
    //cpm::init(argc, argv);
    cpm::InternalConfiguration::init(argc, argv);
    cpm::Writer<VehicleStatePubSubType> writer_vehicleState("vehicleState");
    cpm::Writer<VehicleObservationPubSubType> writer_vehicleObservation("vehicleObservation");
    cpm::Writer<ColorPubSubType> writer_color("color");
    cpm::Writer<VehicleCommandDirectPubSubType> writer_vehCommand("");

    for (size_t i = 0; i < 10; i++)
    {
         {
            VehicleState vehicleState;
            vehicleState.odometer_distance(123);
            vehicleState.vehicle_id(3);
            writer_vehicleState.write(vehicleState);
        }

        {
            VehicleObservation vehicleObservation;
            vehicleObservation.vehicle_id(3);
            
            writer_vehicleObservation.write(vehicleObservation);
        }

        {
            Color c;
            c.a(1);
            c.r(2);
            c.g(3);
            c.b(4);
            
            writer_color.write(c);
        }

        {
            VehicleCommandDirect vehicleCommand;
            vehicleCommand.vehicle_id(3);
            writer_vehCommand.write(vehicleCommand);
        }
        

        usleep(500000);
    }
    
    
}