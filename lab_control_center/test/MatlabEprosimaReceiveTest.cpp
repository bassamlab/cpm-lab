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
#include "cpm/get_topic.hpp"
#include "cpm/get_time_ns.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/dds/HLCHelloPubSubTypes.h"

/**
 * \file VisualizationTest.cpp
 * \brief Test scenario: Creates and sends visualization messages that should be visible in the LCC' MapViewUi
 * \ingroup lcc
 */

int main(int argc, char *argv[]) {
    cpm::init(argc, argv);
    cpm::Logging::Instance().set_id("Logger_test");

    std::cout << "Creating receiver..." << std::endl;

    cpm::ReaderAbstract<HLCHelloPubSubType> hello_receiver("hlc_hello", false, true);
    
    std::cout << "Waiting for a message" << std::flush;
    
    while(! hello_receiver.wait_for_unread_message(100))
    {
        std::cout << "." << std::flush;
    }

    std::cout << "Received message ID was: " << hello_receiver.take().begin()->source_id() << std::endl;

    std::cout << "Shutting down..." << std::endl;

    return 0;
}