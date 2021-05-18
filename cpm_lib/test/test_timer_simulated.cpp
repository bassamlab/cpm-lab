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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
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

#include <unistd.h>
#include <cmath>
#include "TimerSimulated.hpp"
#include "catch.hpp"
#include "cpm/TimerFD.hpp"

#include <string>
#include <thread>
#include <vector>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/Writer.hpp"


#include "cpm/dds/ReadyStatus.h"
#include "cpm/dds/SystemTrigger.h"

/**
 * \test Tests TimerSimulated accuracy
 * 
 * - Does the source ID match the sender ID (data remains unchanged, no other data was received during the test)
 * - Do the ready signals match the expectation (offset and period are correct, increase by period in each step)
 * - Are start signals that do not match the ready signal ignored
 * - Is the current time stamp correct (regarding offset and period)
 * - Does the thread time match the current time (the simulated timestamp should be the same as t_now)
 * \ingroup cpmlib
 */
TEST_CASE("TimerSimulated_accuracy") {
    // Set the Logger ID
    cpm::Logging::Instance().set_id("test_timer_simulated_accuracy");

    const uint64_t period = 21000000;
    const uint64_t offset = 5000000;
    std::string timer_id = "qwertzy";
    cpm::TimerSimulated timer(timer_id, period, offset);

    const int num_runs = 5;

    // Writer to send system triggers to the timer
    cpm::Writer<SystemTriggerPubSubType> writer_SystemTrigger("systemTrigger",
                                                                true);

    // Reader to receive ready signals from the timer
    cpm::ReaderAbstract<ReadyStatusPubSubType> reader_ReadyStatus("readyStatus",
                                                                true);  

    //It usually takes some time for all instances to see each other - wait until then
    std::cout << "Waiting for DDS entity match in Simulated Time test" << std::endl << "\t";
    bool wait = true;
    while (wait)
    {
        usleep(100000); //Wait 100ms
        std::cout << "." << std::flush;

        if (writer_SystemTrigger.matched_subscriptions_size() >= 1 && reader_ReadyStatus.matched_publications_size() >= 1)
            wait = false;
    }
    std::cout << std::endl;                                                          

    /** Test result values **/

    // All ready status signals received from the timer in each run
    std::vector<ReadyStatus> status_ready;

    // Timestamps from timer.get_time() in each call of the callback function
    std::vector<uint64_t> get_time_timestamps;

    // Timestamps t_now in each call of the callback function
    std::vector<uint64_t> t_start_timestamps;

    // Thread that handles the simulated time - it receives
    // ready signals by the timer and sends system triggers
    std::thread signal_thread = std::thread([&]() {

        for (int i = 0; i < num_runs; ++i) {
        // Wait for ready signal
        ReadyStatus status;
        reader_ReadyStatus.wait_for_unread_message(std::numeric_limits<unsigned int>::max());

        status = reader_ReadyStatus.take().at(0);
        status_ready.push_back(status);

        uint64_t next_start = status.next_start_stamp().nanoseconds();

        // Send wrong start signal
        SystemTrigger trigger;
        TimeStamp timestamp;
        timestamp.nanoseconds(next_start - 2);
        trigger.next_start(timestamp);
        writer_SystemTrigger.write(trigger);

        // Send wrong start signal
        timestamp.nanoseconds(next_start - 1);
        trigger.next_start(timestamp);
        writer_SystemTrigger.write(trigger);

        // Wait, no new ready signal should be received
        reader_ReadyStatus.wait_for_unread_message(500);
        // waitset.wait(dds::core::Duration(0, 500000000));

        // Ignore warning that sample is not used
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-but-set-variable"

        if(reader_ReadyStatus.take().size() > 0) 
        {
            // No new sample should be received -> We never want to enter this loop
            std::cout << "THIS SHOULD NOT BE CALLED" << std::endl;
            CHECK(false);
        }
        else
        {
            std::cout << "Iteration okay" << std::endl;
        }

        #pragma GCC diagnostic pop

        // Send correct start signal
        timestamp.nanoseconds(next_start);
        trigger.next_start(timestamp);
        writer_SystemTrigger.write(trigger);
        }

        // Send stop signal - after num_runs, the callback function should not be
        // called again
        std::cout << "Final wait" << std::endl;
        reader_ReadyStatus.wait_for_unread_message(std::numeric_limits<unsigned int>::max());
        std::cout << "Sending stop signals" << std::endl;

        SystemTrigger stop_trigger;
        TimeStamp timestamp;
        timestamp.nanoseconds(cpm::TRIGGER_STOP_SYMBOL);
        stop_trigger.next_start(timestamp);
        writer_SystemTrigger.write(stop_trigger);

        std::cout << "thread terminated" << std::endl;
    });

    int timer_loop_count =
        0;  // timer_loop_count how often the timer callback was called

    // save the timestamps in each run as well as the number of runs
    timer.start([&](uint64_t t_start) {
        get_time_timestamps.push_back(timer.get_time());
        t_start_timestamps.push_back(t_start);

        timer_loop_count++;
    });

    if (signal_thread.joinable()) {
        std::cout << "joining thread" << std::endl;
        signal_thread.join();
    }

    // Checks
    for (size_t i = 0; i < status_ready.size(); ++i) {
        // The source id should always match the id set for the timer
        CHECK(status_ready.at(i).source_id() == timer_id);
        // The ready stamps should match the settings for period and offset
        CHECK(status_ready.at(i).next_start_stamp().nanoseconds() ==
            period * i + offset);
    }

    for (size_t i = 0; i < get_time_timestamps.size(); ++i) {
        // Check if the thread times match the current time and if the current
        // timestamp is correct
        CHECK(t_start_timestamps.at(i) == get_time_timestamps.at(i));
        CHECK((get_time_timestamps.at(i) - offset) % period == 0);
        CHECK(t_start_timestamps.at(i) == i * period + offset);
    }

    // No more than num_runs runs should have taken place
    REQUIRE(timer_loop_count == num_runs);
}
