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

#include "catch.hpp"
#include "cpm/Logging.hpp"
#include "cpm/ParticipantSingleton.hpp"
#include "cpm/RTTTool.hpp"
#include "cpm/dds/RoundTripTimePubSubTypes.h"

#include "cpm/stamp_message.hpp"

#include <mutex>

#include <unistd.h>
#include "cpm/AsyncReader.hpp"
#include "cpm/Writer.hpp"

/**
* \test Tests RTTTool
 * 
 * WARNING: No other participant should be running while this test is running, or it will fail 
 * (due to potential answers to RTT requests by other participants in the network)
 * \ingroup cpmlib
 */
TEST_CASE("RTT") {
    cpm::Logging::Instance().set_id("test_rtt");
    cpm::RTTTool::Instance().activate("test_rtt");

    // Create a reader to check if the message would have been received by the
    // RTTTool async reader (it does not answer, because the ID is the same) Use
    // the reader async
    std::vector<std::string> received_ids;
    std::mutex receive_mutex;
    cpm::AsyncReader<RoundTripTimePubSubType> rtt_reader(
        [&](std::vector<RoundTripTime> &samples) {
            std::lock_guard<std::mutex> lock(receive_mutex);
            for (auto &data : samples) {
            received_ids.push_back(data.source_id());
            }
        },
        "round_trip_time");

    // Create a writer to simulate a RTT request and check if an answer is
    // received with the above reader
    cpm::Writer<RoundTripTimePubSubType> rtt_writer("round_trip_time");

    // It usually takes some time for all instances to see each other - wait until
    // then
    std::cout << "Waiting for DDS entity match in RTT test" << std::endl << "\t";
    bool wait = true;
    while (wait) {
        usleep(100000);  // Wait 100ms
        std::cout << "." << std::flush;

        if (rtt_writer.matched_subscriptions_size() > 1 &&
            rtt_reader.matched_publications_size() > 1)
        wait = false;
    }
    std::cout << std::endl;

    // Now perform testing: Require a RTT measurement and then require a fake one
    // where we should actually expect to receive an answer
    auto rtt_result = cpm::RTTTool::Instance().measure_rtt();

    // Result should be empty, as the measurement should fail
    REQUIRE(rtt_result.size() == 0);

    RoundTripTime fake_request;
    fake_request.count(100);
    fake_request.is_answer(false);
    fake_request.source_id("fake_request");
    rtt_writer.write(fake_request);

    //Wait up to 1 second to check if all sent data was received
    for (int i = 0; i < 10; ++i)
    {
        usleep(100000);

        std::lock_guard<std::mutex> lock(receive_mutex);
        if (received_ids.size() >= 3) break;
    }

    // Now make sure that all required data samples were actually received by the
    // RTT reader
    std::lock_guard<std::mutex> lock(receive_mutex);
    REQUIRE(std::find(received_ids.begin(), received_ids.end(), "test_rtt") !=
            received_ids.end());
    REQUIRE(std::find(received_ids.begin(), received_ids.end(), "fake_request") !=
            received_ids.end());
    REQUIRE(received_ids.size() ==
            3);  // 2x test_rtt (one by measure_rtt(), one by the answer to
                // "fake_request"), 1x "fake_request"
}