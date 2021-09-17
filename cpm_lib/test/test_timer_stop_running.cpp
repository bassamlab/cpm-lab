#include <unistd.h>
#include "catch.hpp"
#include "cpm/TimerFD.hpp"

#include <chrono>
#include <string>
#include <thread>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/Writer.hpp"


#include "ReadyStatus.h"
#include "SystemTrigger.h"

/**
 * \test Tests TimerFD stop signal while running
 * 
 * - Tests if the timer can be stopped by sending a stop signal
 * \ingroup cpmlib
 */
TEST_CASE("TimerFD_stop_signal_when_running") {
    // Set the Logger ID
    cpm::Logging::Instance().set_id("test_timerfd_stop_signal_when_running");

    const uint64_t period = 21000000;
    const uint64_t offset = 5000000;
    cpm::TimerFD timer("xcvbn", period, offset, true);

    // Starting time to check for:
    uint64_t starting_time = timer.get_time() + 3000000000;

    // Writer to send system triggers to the timer
    cpm::Writer<SystemTriggerPubSubType> writer_SystemTrigger("systemTrigger",
                                                                true);

    // Reader to receive ready signals from the timer
    cpm::ReaderAbstract<ReadyStatusPubSubType> reader("readyStatus", true);

    //It usually takes some time for all instances to see each other - wait until then
    std::cout << "Waiting for DDS entity match in Timer Stop Signal While Running test" << std::endl << "\t";
    bool wait = true;
    while (wait)
    {
        usleep(100000); //Wait 100ms
        std::cout << "." << std::flush;

        if (writer_SystemTrigger.matched_subscriptions_size() >= 1 && reader.matched_publications_size() >= 1)
            wait = false;
    }
    std::cout << std::endl;

    // Thread to receive the ready signal, send a start signal and then a stop
    // signal after 100ms
    std::thread signal_thread = std::thread([&]() {
        while (reader.take().size() == 0) {
        usleep(1000);
        continue;
        }

        // Send start signal
        SystemTrigger trigger;
        TimeStamp timestamp;
        timestamp.nanoseconds(starting_time);
        trigger.next_start(timestamp);
        writer_SystemTrigger.write(trigger);

        // Wait
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Send stop signal
        timestamp.nanoseconds(cpm::TRIGGER_STOP_SYMBOL);
        trigger.next_start(timestamp);
        writer_SystemTrigger.write(trigger);
    });

    // Callback function of the timer
    int count =
        0;  // The thread should be stopped before it is called three times

    // Ignore warning that t_start is unused
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"

    timer.start([&](uint64_t t_start) {
        CHECK(count <= 2);  // This task should not be called too often
        usleep(100000);     // simluate variable runtime
        ++count;
    });

    #pragma GCC diagnostic pop

    if (signal_thread.joinable()) {
        signal_thread.join();
    }
}
