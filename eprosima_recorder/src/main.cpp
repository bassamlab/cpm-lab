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

#include <chrono>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "cpm/init.hpp"
#include "cpm/Logging.hpp"

#include "PublishedTopicsListener.hpp"
#include "RecorderManager.hpp"

// For troubleshooting
#include "TopicRecorder.hpp"
#include "cpm/Participant.hpp"
#include "VehicleStatePubSubTypes.h"

/**
 * \brief Main function that sets up the recorder
 * \param argc Part of command line arguments, currently only used by cpm::init
 * \param argv Part of command line arguments, currently only used by cpm::init
 */
int main(int argc, char *argv[]) {
    cpm::init(argc, argv);
    cpm::Logging::Instance().set_id("EprosimaRecorder");

    std::cout << "Initializing..." << std::endl;

    RecorderManager manager("test.txt");

    // cpm::Participant participant(12, false);
    // manager.register_topic_recorder(
    //     "VehicleState", 
    //     "shm_test", 
    //     eprosima::fastdds::dds::WriterQos(), 
    //     participant.get_participant().get()
    // );

    PublishedTopicsListener publisher_listener(12, manager);

    // cpm::Participant participant(12, false);
    // TopicRecorder<VehicleStatePubSubType> recorder("VehicleState", "shm_test", eprosima::fastdds::dds::WriterQos(), participant.get_participant().get());

    std::cout << "Initialization done" << std::endl;

    usleep(5000000);

    std::cout << "Shutting down..." << std::endl;

    return 0;
}