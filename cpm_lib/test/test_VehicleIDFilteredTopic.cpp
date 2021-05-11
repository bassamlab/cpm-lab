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

#include "catch.hpp"
#include "cpm/ParticipantSingleton.hpp"
#include "cpm/VehicleIDFilteredTopic.hpp"
#include "cpm/dds/VehicleStatePubSubTypes.h"
#include "cpm/Reader.hpp"
#include "cpm/Writer.hpp"
#include <unistd.h>

/**
 * \test Tests VehicleIDFilteredTopic
 * 
 * - The filter for vehicle IDs
 * - First sends data for various IDs, then checks if they were received as expected
 * \ingroup cpmlib
 */
TEST_CASE("VehicleIDFilteredTopic") {
  // One writer for all vehicle state test packages
  cpm::Writer<VehicleStatePubSubType> writer_vehicleState("my_topic_name");

  // Reader for state packages with these IDs
  cpm::Reader<VehicleStatePubSubType> reader_vehicle11("my_topic_name", 11);
  cpm::Reader<VehicleStatePubSubType> reader_vehicle42("my_topic_name", 42);

  //It usually takes some time for all instances to see each other - wait until then
    std::cout << "Waiting for DDS entity match in VehicleIDFilteredTopic test" << std::endl << "\t";
    bool wait = true;
    while (wait)
    {
        usleep(10000); //Wait 10ms
        std::cout << "." << std::flush;

        if  (writer_vehicleState.matched_subscriptions_size() > 0 && 
                reader_vehicle11.matched_publications_size() >=1 && 
                reader_vehicle42.matched_publications_size() >= 1
            )
            wait = false;
    }
    std::cout << std::endl;

  // send state packages with different IDs, also those that should be ignored
  {
    VehicleState vehicleState;
    vehicleState.odometer_distance(123);
    vehicleState.vehicle_id(3);
    writer_vehicleState.write(vehicleState);
  }

  {
    VehicleState vehicleState;
    vehicleState.odometer_distance(2);
    vehicleState.vehicle_id(42);
    writer_vehicleState.write(vehicleState);
  }

  {
    VehicleState vehicleState;
    vehicleState.odometer_distance(3);
    vehicleState.vehicle_id(11);
    writer_vehicleState.write(vehicleState);
  }

  {
    VehicleState vehicleState;
    vehicleState.odometer_distance(6);
    vehicleState.vehicle_id(42);
    writer_vehicleState.write(vehicleState);
  }

  // wait for 'transmission' for up to 1 second
    std::vector<VehicleState> reader_samples11;
    std::vector<VehicleState> reader_samples42;
    for (int i = 0; i < 10; ++i)
    {
        auto reader_samples11_dds = reader_vehicle11.get_all_samples();
        auto reader_samples42_dds = reader_vehicle42.get_all_samples();

        //Abort early if condition is fulfilled, else wait and repeat read
        if (reader_samples11.size() >=1 && reader_samples42.size() >= 2) break;
        else usleep(100000);
    }

  REQUIRE(reader_samples11.size() == 1);
  REQUIRE(reader_samples42.size() == 2);

  REQUIRE(reader_samples42[0].vehicle_id() == 42);
  REQUIRE(reader_samples42[0].odometer_distance() == 2);

  REQUIRE(reader_samples42[1].vehicle_id() == 42);
  REQUIRE(reader_samples42[1].odometer_distance() == 6);

  REQUIRE(reader_samples11[0].vehicle_id() == 11);
  REQUIRE(reader_samples11[0].odometer_distance() == 3);
}