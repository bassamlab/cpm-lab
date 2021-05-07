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

/**
 * \file main.cpp
 * \brief This file includes a reader that receives NUC messages
 */

#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <functional>

#include "cpm/dds/HLCHelloPubSubTypes.h"

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/get_topic.hpp"
#include "cpm/Logging.hpp"
#include "cpm/CommandLineReader.hpp"
#include "cpm/init.hpp"
#include "cpm/AsyncReader.hpp"

int main (int argc, char *argv[]) { 
    //Initialize the cpm logger, set domain id etc
    cpm::init(argc, argv);
    cpm::Logging::Instance().set_id("hlc_hello");

    //Create DataReader that reads NUC ready messages
    cpm::AsyncReader<HLCHelloPubSubType> reader(
        [](std::vector<HLCHello>& samples){
            for (auto& data : samples)
            {
                std::cout << "Received: " << data.source_id() << " " << data.script_running() << " " << data.middleware_running() << std::endl;
            }
        },
        "hlc_hello",
        true
    );

    std::cout << "Press Enter to stop the program" << std::endl;
    std::cin.get();

    return 0;
}