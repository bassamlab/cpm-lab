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