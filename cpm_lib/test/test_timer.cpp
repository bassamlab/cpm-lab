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
#include "cpm/TimerFD.hpp"

#include <thread>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/Writer.hpp"
#include "cpm/get_topic.hpp"

#include <cpm/dds/ReadyStatus.h>
#include <cpm/dds/SystemTrigger.h>

#include <climits>

#include <cpm/ReaderAbstract.hpp>

/**
 * \test Tests TimerFD accuracy
 * 
 * - Is the timer started after the initial starting time
 * - Does t_now match the expectation regarding offset, period and start values
 * - Is the callback function called shortly after t_now
 * - Is the timer actually stopped when it should be stopped
 * - If the callback function takes longer than period to finish, is this handled correctly
 * \ingroup cpmlib
 */
TEST_CASE("TimerFD_accuracy") {
  std::cout << "TimerFD_accuracy" << std::endl;
  // Set the Logger ID
  cpm::Logging::Instance().set_id("test_timerfd_accuracy");

  const uint64_t period = 21000000;
  const uint64_t offset = 5000000;

  const std::string time_name = "asdfg";

  cpm::TimerFD timer(time_name, period, offset, true);

  // Starting time to check for:
  uint64_t starting_time = timer.get_time() + 3000000000;

  // Writer to send system triggers to the timer
  cpm::Writer<SystemTriggerPubSubType> timer_system_trigger_writer(
      "systemTrigger", true);
  
  // Reader to receive ready signals from the timer
  cpm::ReaderAbstract<ReadyStatusPubSubType> reader("readyStatus", true);

  //It usually takes some time for all instances to see each other - wait until then
  std::cout << "Waiting for DDS entity match in Timer Accuracy test" << std::endl << "\t";
  bool wait = true;
  while (wait)
  {
      usleep(100000); //Wait 100ms
      std::cout << "." << std::flush;

      if (timer_system_trigger_writer.matched_subscriptions_size() >= 1 && reader.matched_publications_size() >= 1)
          wait = false;
  }
  std::cout << std::endl;

  // Variables for CHECKs - only to identify the timer by its id
  std::string source_id;

  // Thread to receive the ready signal and send a start signal afterwards
  std::thread signal_thread = std::thread([&]() {
    reader.wait_for_unread_message(std::numeric_limits<unsigned int>::max());
    source_id = reader.take().front().source_id();
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

  timer.start([&](uint64_t t_start) {
      uint64_t now = timer.get_time();

      //Curent timer should match the expectation regarding starting time and period
      CHECK( now >= starting_time + period * timer_loop_count); 

      if (timer_loop_count == 0) {
          // actual start time is within 1 ms of initial start time
          CHECK( t_start <= starting_time + period + 1000000); 
      }
      CHECK( t_start <= now ); //Callback should not be called before t_start
      CHECK( now <= t_start + 1000000 ); // actual start time is within 1 ms of declared start time
      CHECK( t_start % period == offset ); // start time corresponds to timer definition

      if(timer_loop_count > 0)
      {
          //Fitting to the sleep behaviour, the difference between the periods should match this expression
          CHECK( ((timer_loop_count%3)+1)*period == t_start - t_start_prev); 
      }

      timer_loop_count++;
      if(timer_loop_count > 15) {
          timer.stop();
      }

      t_start_prev = t_start;

      // simluate variable runtime that can be greater than period
      usleep( ((timer_loop_count%3)*period + period/3) / 1000 ); 

  });

  if (signal_thread.joinable()) {
    signal_thread.join();
  }

  // Check that the ready signal matches the expected ready signal
  CHECK(source_id == time_name);
}
