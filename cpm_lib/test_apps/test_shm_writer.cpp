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

int main() {
    cpm::Logging::Instance().set_id("test_writer");

    cpm::Participant shm_participant(12, true);

    // Test the writer, find out if sample gets received
    cpm::Writer<VehicleStatePubSubType> vehicle_state_writer(
        shm_participant.get_participant(),
        "shm_test",
        true, 
        true, 
        true
    );

    // Send sample
    VehicleState vehicleState;
    vehicleState.vehicle_id(99);

    for (int i = 0; i < 1000000; ++i) {
        vehicle_state_writer.write(vehicleState);
        sleep(0.01);
    }
}