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
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
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

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_set>

#include "TopicRecorder.hpp"

// Include all IDL types that need to be supported here
#include "VehicleStatePubSubTypes.h"

/**
 * \class RecorderManager
 * \brief Data class that holds all TopicRecorders. Can be used to register new recorders.
 */
class RecorderManager {
private:
    //! Holds all currently registered recorders
    std::vector<std::shared_ptr<AbstractTopicRecorder>> topic_recorders;

    //! Holds all already registered topic names
    std::unordered_set<std::string> registered_topics;

public:
    /**
     * \brief Constructor. Allows to set the filename to output the recorded messages to.
     * \param output_filename Filename to output the messages to.
     */
    RecorderManager(std::string output_filename) 
    {

    }

    /**
     * \brief Registers a new topic recorder, given that the topic name does not already exist
     * \param topic_type Name of the topic type
     * \param topic_name Name of the topic
     * \param publisher_qos QoS settings for the publisher
     * \param participant Domain participant to use for publisher etc. creation
     */
    void register_topic_recorder(
        std::string topic_type, 
        std::string topic_name, 
        eprosima::fastdds::dds::WriterQos publisher_qos, 
        eprosima::fastdds::dds::DomainParticipant* participant
    )
    {
        // Don't create a new recorder if one already exists
        if (registered_topics.find(topic_name) != registered_topics.end())
        {
            return;
        }

        // Create a new recorder and store it in the vector to keep it alive
        // Go through all possible IDL types until the right type for recorder creation is found
        if (topic_type.compare("VehicleState") == 0)
        {
            topic_recorders.emplace_back(
                new TopicRecorder<VehicleStatePubSubType>(topic_type, topic_name, publisher_qos, participant)
            );
        }

        registered_topics.insert(topic_name);
    }
};