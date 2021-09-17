#include <unistd.h>
#include "catch.hpp"
#include "cpm/SimpleTimer.hpp"

#include <thread>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/Writer.hpp"


#include <cpm/dds/ReadyStatus.h>
#include <cpm/dds/SystemTrigger.h>

/**
 * \test Tests SimpleTimer with curstom stop signal
 * 
 * - Sends a custom stop signal and checks whether it works
 * - Therefore: Makes sure that the timer callback function is never actually called
 * \ingroup cpmlib
 */
TEST_CASE("SimpleTimer_custom_stop_signal") {
    // Set the Logger ID
    cpm::Logging::Instance().set_id("test_simple_timer_custom_stop_signal");

    const uint64_t period_ms = 100;
    bool wait_for_start = true;
    bool react_to_stop = true;
    uint64_t custom_stop_signal = 1234;
    std::string timer_id = "0";
    cpm::SimpleTimer timer(timer_id, period_ms, wait_for_start, react_to_stop,
                            custom_stop_signal);

    // Writer to send system triggers to the timer
    cpm::Writer<SystemTriggerPubSubType> writer_SystemTrigger("systemTrigger",
                                                                true);
    // Reader to receive ready signals from the timer
    cpm::ReaderAbstract<ReadyStatusPubSubType> reader("readyStatus", true);
    
    //It usually takes some time for all instances to see each other - wait until then
    std::cout << "Waiting for DDS entity match in Timer Stop test" << std::endl << "\t";
    bool wait = true;
    while (wait)
    {
        usleep(100000); //Wait 100ms
        std::cout << "." << std::flush;

        if (writer_SystemTrigger.matched_subscriptions_size() >= 1 && reader.matched_publications_size() >= 1)
            wait = false;
    }
    std::cout << std::endl;

    // Thread to send a stop signal after the ready signal was received
    std::thread signal_thread = std::thread([&]() {

        reader.wait_for_unread_message(std::numeric_limits<unsigned int>::max());
        // Send stop signal
        SystemTrigger trigger;
        TimeStamp timestamp;
        timestamp.nanoseconds(custom_stop_signal);
        trigger.next_start(timestamp);
        writer_SystemTrigger.write(trigger);
    });

    // Ignore warning that t_start is unused
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"

    timer.start([&](uint64_t t_start) {
        // The timer should never start because it is stopped before that can happen
        // (No start signal is sent)
        CHECK(false);
    });

    // This part can only be reached if the timer was stopped

    #pragma GCC diagnostic pop

    if (signal_thread.joinable()) {
        signal_thread.join();
    }
}
