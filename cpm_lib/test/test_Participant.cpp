#include <unistd.h>
#include "catch.hpp"
#include "cpm/Logging.hpp"
#include "cpm/ParticipantSingleton.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/dds/VehicleStatePubSubTypes.h"
#include "cpm/stamp_message.hpp"

#include "cpm/Participant.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/Writer.hpp"

/**
 * \test Tests Participant
 * 
 * - Can QOS XML files be read
 * - Does the participant work with ReaderAbstract and Writer
 * \ingroup cpmlib
 */
TEST_CASE("Participant") {
    cpm::Logging::Instance().set_id("test_participant");

    cpm::Participant p(
        5, "../test/QOS_TEST.xml");  // The path depends on from where the program
                                    // is called

    {
        cpm::ReaderAbstract<VehicleStatePubSubType> vehicle_state_reader(
            p.get_participant(), "sadfhasdflkasdhf", true, true, true);

        // Test the participant, find out if sample gets received
        cpm::Writer<VehicleStatePubSubType> vehicle_state_writer(
            p.get_participant(), "sadfhasdflkasdhf", true, true, true);

        //It usually takes some time for all instances to see each other - wait until then
        std::cout << "Waiting for DDS entity match in Participant test" << std::endl << "\t";
        bool wait = true;
        while (wait)
        {
            usleep(10000); //Wait 10ms
            std::cout << "." << std::flush;

            if (vehicle_state_writer.matched_subscriptions_size() > 0 && vehicle_state_reader.matched_publications_size() > 0)
                wait = false;
        }
        std::cout << std::endl;
        
        // Send sample
        VehicleState vehicleState;
        vehicleState.vehicle_id(99);
        vehicle_state_writer.write(vehicleState);

        //Receive sample, maybe multiple times because this behaviour is not deterministic
        //Thus, for VMs, slow machines etc, wait up to 1 second before failing this test
        auto samples = vehicle_state_reader.take();
        for (int i = 0; i < 9; ++i)
        {
            if (samples.size() <= 0)
            {
                usleep(100000);
                samples = vehicle_state_reader.take();
            }
            else break;
        }

        // Check that sample content is correct
        REQUIRE(samples.size() == 1);
        REQUIRE(samples.begin()->vehicle_id() == 99);
    }
}