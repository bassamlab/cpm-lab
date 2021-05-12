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
#include "catch.hpp"
#include "cpm/SimpleTimer.hpp"

#include <thread>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/Writer.hpp"
#include "cpm/get_topic.hpp"

#include <cpm/dds/ReadyStatus.h>
#include <cpm/dds/SystemTrigger.h>

#include "cpm/dds/ReadyStatus.h"
#include "cpm/dds/SystemTrigger.h"

/**
 * \test Tests SimpleTimer
 * 
 * - Is the timer started after the initial starting time
 * - Does t_now match the expectation regarding offset, period and start values
 * - Is the callback function called shortly after t_now
 * - Is the timer actually stopped when it should be stopped
 * - If the callback function takes longer than period to finish, is this handled correctly
 * \ingroup cpmlib
 */

TEST_CASE("SimpleTimer functionality") {
  // Set the Logger ID
  cpm::Logging::Instance().set_id("test_simple_timer");

  const uint64_t period = 100;  // every 100ms

  const std::string time_name = "asdfg";

  cpm::SimpleTimer timer(time_name, period, true);

  // Starting time to check for:
  uint64_t starting_time = timer.get_time() + 2000000000;

  // Writer to send system triggers to the timer
  cpm::Writer<SystemTriggerPubSubType> timer_system_trigger_writer(
      "systemTrigger", true);

  // Reader to receive ready signals from the timer
  cpm::ReaderAbstract<ReadyStatusPubSubType> reader("readyStatus", true);

  // Variables for CHECKs - only to identify the timer by its id
  std::string source_id;

  //It usually takes some time for all instances to see each other - wait until then
  std::cout << "Waiting for DDS entity match in Simple Timer test" << std::endl << "\t";
  bool wait = true;
  while (wait)
  {
      usleep(100000); //Wait 100ms
      std::cout << "." << std::flush;

      if (timer_system_trigger_writer.matched_subscriptions_size() >= 1 && reader.matched_publications_size() >= 1)
          wait = false;
  }
  std::cout << std::endl;

  // Thread to receive the ready signal and send a start signal afterwards
  std::thread signal_thread = std::thread([&]() {
    reader.wait_for_unread_message(std::numeric_limits<unsigned int>::max());
    for (auto& data : reader.take())
    {
      source_id = data.source_id();
    }

    // Send start signal
    SystemTrigger trigger;
    TimeStamp timestamp;
    timestamp.nanoseconds(starting_time);
    trigger.next_start(timestamp);
    timer_system_trigger_writer.write(trigger);
  });

  // Variables for the callback function
  int timer_loop_count = 0;
  uint64_t t_start_prev = 0;

  uint64_t period_ns = period * 1000000;

  timer.start([&](uint64_t t_start) {
    uint64_t now = timer.get_time();

    // Curent timer should match the expectation regarding starting time and
    // period
    CHECK(now >= starting_time + period_ns * timer_loop_count);

    if (timer_loop_count == 0) {
      // actual start time is within 2 ms of initial start time
      CHECK(t_start <= starting_time + period_ns + 2000000);
    }

    timer_loop_count++;
    if (timer_loop_count > 15) {
      timer.stop();
    }

    t_start_prev = t_start;

    // simluate variable runtime that can be greater than period
    usleep(((timer_loop_count % 3) * period_ns + period_ns / 3) / 1000);
  });

  if (signal_thread.joinable()) {
    signal_thread.join();
  }

  // Check that the ready signal matches the expected ready signal
  CHECK(source_id == time_name);

  std::cout << "out" << std::endl;
}
