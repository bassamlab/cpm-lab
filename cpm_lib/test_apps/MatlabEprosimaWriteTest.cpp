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
#include "cpm/dds/VehicleStateListPubSubTypes.h"
#include "cpm/dds/SystemTriggerPubSubTypes.h"

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

    std::shared_ptr<cpm::Participant> particpant_ptr = std::make_shared<cpm::Participant>(2, true);
    cpm::Writer<SystemTriggerPubSubType> writer(particpant_ptr->get_participant(), "systemTrigger", true, true, true);
    //cpm::Writer<VehicleStateListPubSubType> writer(particpant_ptr->get_participant(), "vehicleStateList", false, false, false);
    
    while(writer.matched_subscriptions_size() < 1)
    {
        std::cout << "." << std::flush;
        usleep(50000);
    }

    //Create some fictional VehicleStateList data for testing purposes
    VehicleStateList vehicle_state_list;
    vehicle_state_list.active_vehicle_ids({1, 3, 7});
    vehicle_state_list.t_now(5);

    VehicleState state1;
    state1.battery_voltage(70);

    VehicleState state2;
    state2.battery_voltage(80);

    vehicle_state_list.state_list({ state1, state2 });

    VehicleObservation observation1;
    observation1.vehicle_id(7);

    vehicle_state_list.vehicle_observation_list({ observation1 });

    SystemTrigger trigger;
    TimeStamp stamp;
    stamp.nanoseconds(50);
    trigger.next_start(stamp);
    
    //Write the message three times
    auto repeat = 3;
    for (auto i = 0; i < repeat; ++i)
    {
        //writer.write(vehicle_state_list);
        writer.write(trigger);
        //std::cout << trigger.next_start().nanoseconds() << std::endl;
        usleep(100000);
    }

    std::cout << "Shutting down..." << std::endl;

    return 0;
}