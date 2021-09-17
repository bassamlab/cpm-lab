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