#include "cpm/Logging.hpp"
#include "cpm/ParticipantSingleton.hpp"
#include "cpm/dds/VehicleStatePubSubTypes.h"
#include "cpm/stamp_message.hpp"

#include "cpm/ReaderAbstract.hpp"
#include "cpm/Writer.hpp"

#include <unistd.h>

/**
 * Tests:
 * - The cpm reader
 * - If the reader returns the newest valid sample
 */

using namespace std::placeholders;
int main() {
    cpm::Logging::Instance().set_id("test_reader");

    cpm::Participant shm_participant(12, true);

    //Test the writer, find out if sample gets received
    cpm::ReaderAbstract<VehicleStatePubSubType> vehicle_state_reader(
        shm_participant.get_participant(),
        "shm_test",
        true, 
        true, 
        true
    );
    
    // cpm::ReaderParent<VehicleStatePubSubType> r_parent(
    //     [] (std::vector<VehicleState>& vec) {
    //         std::cout << "Yay callback" << std::endl;
    //     },
    //     shm_participant.get_participant(),
    //     "shm_test", 
    //     true,
    //     true,
    //     true
    // );

    std::cout << "Waiting for samples" << std::endl;

    for (int i = 0; i < 5; ++i)
    {
        std::cout << "." << std::flush;
        sleep(1);
    }

    std::cout << "Stopped waiting" << std::endl;

    // Wait for sample
    // while (true)
    // {
    //     // Receive sample
    //     auto samples = vehicle_state_reader.take();

    //     if (samples.size() > 0)
    //     {
    //         std::cout << std::endl << "Sample received! ID is: ";
    //         std::cout << static_cast<int>(samples.front().vehicle_id()) << std::endl;
    //         return 0;
    //     }

    //     std::cout << ".";
    //     sleep(0.1);
    // }
}