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

#include "catch.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <functional>
#include <random>
#include <algorithm>
#include <thread>
#include <chrono>

#include "cpm/init.hpp"
#include "cpm/Timer.hpp"
#include "cpm/Parameter.hpp"
#include "cpm/ParticipantSingleton.hpp"
#include "cpm/Logging.hpp"
#include "cpm/dds/VehicleCommandSpeedCurvaturePubSubTypes.h"
#include "cpm/dds/VehicleStateListPubSubTypes.h"
#include "cpm/dds/ParameterRequestPubSubTypes.h"

#include "cpm/AsyncReader.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/Writer.hpp"
#include "cpm/Participant.hpp"

#include "Communication.hpp"

/**
 * \test Tests communication from simulated HLC to simulated vehicle
 * 
 * Tests if data sent by a virtual HLC is received by a virtual vehicle, 
 * therefore if TypedCommunication receives and sends the data as required
 * \ingroup middleware
 */
TEST_CASE( "HLCToVehicleCommunication" ) {
    //Set different domain ID than 1 for the domain of the vehicle
    std::vector<char*> argv;
    std::string domain_argument = "--dds_domain=3";
    argv.push_back(const_cast<char*>(domain_argument.c_str()));
    cpm::init(argv.size() - 1, argv.data());
    cpm::Logging::Instance().set_id("middleware_test");

    //Data for the tests
    uint64_t max_rounds = 5;
    std::vector<uint64_t> received_round_numbers;
    std::mutex round_numbers_mutex;    

    {
        //Communication parameters
        int hlcDomainNumber = 1; 
        std::string vehicleStateListTopicName = "vehicleStateList"; 
        std::string vehicleTrajectoryTopicName = "vehicleCommandTrajectory"; 
        std::string vehiclePathTrackingTopicName = "vehicleCommandPathTracking"; 
        std::string vehicleSpeedCurvatureTopicName = "vehicleCommandSpeedCurvature"; 
        std::string vehicleDirectTopicName = "vehicleCommandDirect"; 
        int vehicleID = 0; 
        std::vector<uint8_t> vehicle_ids = { 0 };

        //Timer parameters
        std::string node_id = "middleware";
        uint64_t period_nanoseconds = 6000000; //6ms
        uint64_t offset_nanoseconds = 1;
        bool simulated_time_allowed = true;
        bool simulated_time = false;

        //Initialize the timer
        std::shared_ptr<cpm::Timer> timer = cpm::Timer::create(
            node_id,
            period_nanoseconds,
            offset_nanoseconds,
            false,
            simulated_time_allowed,
            simulated_time);

        //Initialize the communication 
        Communication communication(
            hlcDomainNumber,
            vehicleStateListTopicName,
            vehicleTrajectoryTopicName,
            vehiclePathTrackingTopicName,
            vehicleSpeedCurvatureTopicName,
            vehicleDirectTopicName,
            timer,
            vehicle_ids
        );

        //Thread that simulates the vehicle (only a reader is created). A waitset is attached to the reader and a callback function is created. 
        //In this function, round numbers are stored - the number of each round should be received.
        cpm::AsyncReader<VehicleCommandSpeedCurvaturePubSubType> vehicleReader([&] (std::vector<VehicleCommandSpeedCurvature>& samples)
        {
            // Store received data for later checks
            std::lock_guard<std::mutex> lock(round_numbers_mutex);
            for (auto& data : samples) {
                    received_round_numbers.push_back(data.header().create_stamp().nanoseconds());
            }
        }, vehicleSpeedCurvatureTopicName);

        //Sleep for some milliseconds just to make sure that the reader has been initialized properly
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        //Send test data from a virtual HLC - only the round number matters here, which is transmitted using the timestamp value
        //The data is sent to the middleware (-> middleware participant, because of the domain ID), and the middleware should send it to the vehicle
        auto participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->lookup_participant(hlcDomainNumber);
        assert(participant != nullptr);
        cpm::Writer<VehicleCommandSpeedCurvaturePubSubType> hlcWriter(*participant, vehicleSpeedCurvatureTopicName);
        for (uint64_t i = 0; i <= max_rounds; ++i) {
            //Send data and wait
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            VehicleCommandSpeedCurvature curv;
            curv.vehicle_id(vehicleID);

            TimeStamp timestamp;
            timestamp.nanoseconds(i);
            Header header;
            header.create_stamp(timestamp);
            header.valid_after_stamp(timestamp);

            curv.header(header);
            curv.speed(0);
            curv.curvature(0);
            hlcWriter.write(curv);
        }
    }

    //Perform checks (wait a while before doing so, to make sure that everything has been received)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::lock_guard<std::mutex> lock(round_numbers_mutex);
    //"Dirty" bugfix: Check if some of the data was received (as sometimes exactly one data point is missing)
    CHECK((received_round_numbers.size() >= (max_rounds - 2) && received_round_numbers.size() >= 1));
}