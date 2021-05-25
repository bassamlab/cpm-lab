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
#include "cpm/ParticipantSingleton.hpp"

#include "cpm/get_time_ns.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/Writer.hpp"
#include "cpm/dds/VisualizationPubSubTypes.h"
#include "cpm/dds/ReadyStatusPubSubTypes.h"

/**
 * \file VisualizationTest.cpp
 * \brief Test scenario: Creates and sends visualization messages that should be visible in the LCC' MapViewUi
 * \ingroup lcc
 */

int main(int argc, char *argv[]) {
    cpm::init(argc, argv);
    cpm::Logging::Instance().set_id("Logger_test");

    std::cout << "Creating receiver..." << std::endl;

    // std::shared_ptr<cpm::Participant> particpant_ptr = std::make_shared<cpm::Participant>(0);
    // cpm::ReaderAbstract<VisualizationPubSubType> reader(particpant_ptr->get_participant(), "visualization");

    std::shared_ptr<cpm::Participant> particpant_ptr = std::make_shared<cpm::Participant>(1, true);
    cpm::ReaderAbstract<ReadyStatusPubSubType> reader(particpant_ptr->get_participant(), "readyStatus", true, true, true);
    
    while(! reader.wait_for_unread_message(100))
    {
        std::cout << "." << std::flush;
    }
    std::cout << std::endl;

    auto result = reader.take();

    std::cout 
        << "Received messages: ID is " 
        << result.begin()->source_id() 
        << ", and the inital time stamp is "
        << result.begin()->next_start_stamp().nanoseconds()
        << std::endl;

    std::cout << "Shutting down..." << std::endl;

    return 0;
}