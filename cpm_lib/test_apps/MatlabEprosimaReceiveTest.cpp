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
#include "cpm/dds/VehicleCommandTrajectoryPubSubTypes.h"

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

    // std::shared_ptr<cpm::Participant> particpant_ptr = std::make_shared<cpm::Participant>(2, true);
    // cpm::ReaderAbstract<VehicleCommandTrajectoryPubSubType> reader(particpant_ptr->get_participant(), "vehicleCommandTrajectory", false, false, false);

    std::shared_ptr<cpm::Participant> particpant_ptr = std::make_shared<cpm::Participant>(2, true);
    cpm::ReaderAbstract<ReadyStatusPubSubType> reader(particpant_ptr->get_participant(), "readyStatus", true, true, true);
    
    while(! reader.wait_for_unread_message(100))
    {
        std::cout << "." << std::flush;
    }
    std::cout << std::endl;

    auto result = reader.take();

    // std::cout 
    //     << "Received messages: ID is " 
    //     << static_cast<int>(result.begin()->vehicle_id())
    //     << ", and the inital px is "
    //     << result.begin()->trajectory_points().begin()->px()
    //     << std::endl;

    std::cout 
        << "Received messages: ID is " 
        << result.begin()->source_id()
        << ", and the time stamp is "
        << result.begin()->next_start_stamp().nanoseconds()
        << std::endl;

    std::cout << "Shutting down..." << std::endl;

    return 0;
}