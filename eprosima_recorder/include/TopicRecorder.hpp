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

#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>

//! Helper class to be able to store all topic recorders in a single vector
class AbstractTopicRecorder {};

/**
 * \class TopicRecorder
 * \brief This class is very close to ReaderParent in the cpm lib, but minor different changes in requirements 
 * (e.g. using a pointer to a dds participant) and in general less requirements regarding functionality lead
 * to the current decision of creating a slightly different version
 */ 
template<class MessageType>
class TopicRecorder : public AbstractTopicRecorder
{
private:
    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:
        /**
         * \brief Constructor, init. active_matches count & register the callback
         * \param _registered_callback The callback that gets called whenever new messages are available.
         * It gets passed all new message data.
         */
        SubListener(std::function<void(std::vector<typename MessageType::type>&)> _registered_callback)
            : registered_callback(_registered_callback)
        {
        }

        //! Destructor
        ~SubListener() override
        {
            std::cout << "Destructed!" << std::endl;
        }

        /**
         * \brief Called whenever a new match (e.g. to a writer) takes place
         * \param reader Unused, but must be given anyway to override
         * \param info Used to get total count of current matches / matched writers
         */
        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader*,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override
        {
            std::cout << "You've got a match! (" << info.total_count << ")" << std::endl;
        }

        /**
         * \brief Called whenever new data is available. Calls the callback function of ReaderParent (registered_callback) if available
         * \param reader The reader to read new data from
         */
        void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override 
        {
            std::cout << "You've got data!" << std::endl;

            std::vector<typename MessageType::type> buffer;          

            eprosima::fastdds::dds::SampleInfo info;
            typename MessageType::type data;
            while(reader->take_next_sample(&data, &info) == ReturnCode_t::RETCODE_OK)
            {
                if (info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
                {
                    buffer.push_back(data);
                }
            }

            if(buffer.size() > 0 && registered_callback){
                registered_callback(buffer);
            }
        }

        void on_sample_rejected(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastrtps::SampleRejectedStatus& status) override
        {
            std::cout << "SAMPLE REJECTED" << std::endl;
        }

        /**
         * @brief Method called an incompatible QoS was requested.
         * @param reader The DataReader
         * @param status The requested incompatible QoS status
         */
        void on_requested_incompatible_qos(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::RequestedIncompatibleQosStatus& status) override
        {
            std::cout << "QOS INCOMPATIBLE" << std::endl;
        }

        /**
         * @brief Method called when a sample was lost.
         * @param reader The DataReader
         * @param status The sample lost status
         */
        void on_sample_lost(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SampleLostStatus& status) override
        {
            std::cout << "SAMPLE LOST" << std::endl;
        }

        //! Callback function to be called whenever messages get received, takes std::vector of messages as argument, is void
        std::function<void(std::vector<typename MessageType::type>&)> registered_callback;

    } listener;

    std::shared_ptr<eprosima::fastdds::dds::Subscriber> sub;
    std::shared_ptr<eprosima::fastdds::dds::DataReader> reader;
    std::shared_ptr<eprosima::fastdds::dds::TopicDescription> topic;
    eprosima::fastdds::dds::TypeSupport type_support;

    void callback(std::vector<typename MessageType::type>& samples) {
        std::cout << "Received something!" << std::endl;
    }

public: 
    /**
     * \brief Creates a subscriber from minimal topic information, publisher QoS and a domain participant
     * \param topic_type Name of the topic type
     * \param topic_name Name of the topic
     * \param publisher_qos QoS settings for the publisher
     * \param participant Domain participant to use for publisher etc. creation
     */
    TopicRecorder(
        std::string topic_type, 
        std::string topic_name, 
        eprosima::fastdds::dds::WriterQos publisher_qos, 
        eprosima::fastdds::dds::DomainParticipant* participant
    );

    TopicRecorder();
};

using namespace std::placeholders;

template<class MessageType> 
TopicRecorder<MessageType>::TopicRecorder() :
    type_support(new MessageType()), listener(std::bind(&TopicRecorder::callback, this, _1))
{
    
}

template<class MessageType> 
TopicRecorder<MessageType>::TopicRecorder(
    std::string topic_type, 
    std::string topic_name, 
    eprosima::fastdds::dds::WriterQos publisher_qos, 
    eprosima::fastdds::dds::DomainParticipant* participant
) :
    type_support(new MessageType()), listener(std::bind(&TopicRecorder::callback, this, _1))
{
    std::cout << "Creating recorder for topic type " << topic_type << " with name " << topic_name << std::endl;

    // TODO use participant QoSs
    sub = std::shared_ptr<eprosima::fastdds::dds::Subscriber>(
        participant->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT),
        [&] (eprosima::fastdds::dds::Subscriber* sub)
        {
            if (sub != nullptr)
            {
                participant->delete_subscriber(sub);
            }
        }
    );
    assert(sub);

    // Check if Type is already registered, create type
    auto find_type_ret = participant->find_type(topic_type);
    if(find_type_ret.empty()){
        auto ret = type_support.register_type(participant);
        assert(ret == eprosima::fastdds::dds::TypeSupport::ReturnCode_t::RETCODE_OK);
    }

    assert(participant->find_type(topic_type).empty() == false);

    // Create Topic
    auto find_topic = participant->lookup_topicdescription(topic_name);
    if(find_topic == nullptr){
        std::string type_name_str = topic_type;
        topic = std::shared_ptr<eprosima::fastdds::dds::Topic>(
            participant->create_topic(topic_name, type_name_str, eprosima::fastdds::dds::TOPIC_QOS_DEFAULT),
            [&](eprosima::fastdds::dds::Topic* topic) {
                if (topic != nullptr)
                {
                    participant->delete_topic(topic);
                }
            }
        );
    }else{
        topic = std::shared_ptr<eprosima::fastdds::dds::Topic>(
            (eprosima::fastdds::dds::Topic*)find_topic,
            [&](eprosima::fastdds::dds::Topic* topic) {
                if (topic != nullptr)
                {
                    participant->delete_topic(topic);
                }
            }
        );
    }

    assert(topic);

    // Create Reader. TODO: Find a way to figure out DataReaderQoS
    eprosima::fastdds::dds::DataReaderQos data_reader_qos;
    auto policy = eprosima::fastdds::dds::ReliabilityQosPolicy();
    policy.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    data_reader_qos.reliability(policy);
    reader = std::shared_ptr<eprosima::fastdds::dds::DataReader>(
        sub->create_datareader(topic.get(), data_reader_qos, &listener),
        [&](eprosima::fastdds::dds::DataReader* reader) {
            if (sub != nullptr)
            {
                sub->delete_datareader(reader);
            }
        }
    );
    assert(reader);   
}