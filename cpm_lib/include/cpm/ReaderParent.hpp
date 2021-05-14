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

#include <iostream>
#include <algorithm>
#include <string>
#include <functional>
#include <vector>
#include <future>

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/Participant.hpp"

namespace cpm 
{
    /**
     * \class ReaderParent.hpp
     * \brief This class is used by all other reader functions to implement their own specific reader functionality.
     * The class itself is not supposed to be used as such.
     */ 

    template<class MessageType> 
    class ReaderParent
    {
    private:
        //Reader and waitset for receiving data and calling the callback function
        //! eProsima subscriber
        std::shared_ptr<eprosima::fastdds::dds::Subscriber> sub;
        //! eProsima reader, which is used to read received data
        std::shared_ptr<eprosima::fastdds::dds::DataReader> reader;
        //! eProsima topic, the topic that the reader is supposed to use
        std::shared_ptr<eprosima::fastdds::dds::TopicDescription> topic;
        //! eProsima type support to create the right topic type
        eprosima::fastdds::dds::TypeSupport type_support;

        //! eProsima participant used, may be "external", e.g. coming from ParticipantSingleton
        std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant_;

        //! Data type of the message / used for the topic
        MessageType topic_data_type;

        //! Topic name, e.g. "system_trigger"
        std::string topic_name;

        //! Listener class that invokes the callback function whenever data is received & counts the number of matched subscriptions
        class SubListener : public eprosima::fastdds::dds::DataReaderListener
        {
        public:
            //! Constructor, init. active_matches count
            SubListener(std::function<void(std::vector<typename MessageType::type>&)> _registered_callback)
                : active_matches(0), registered_callback(_registered_callback)
            {
            }

            //! Destructor
            ~SubListener() override
            {
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
                active_matches = info.total_count;
            }

            /**
             * \brief Called whenever new data is available. Calls the callback function of ReaderParent (registered_callback) if available
             * \param reader The reader to read new data from
             */
            void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override 
            {
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

            //! Counts the matches of the data reader
            std::atomic_int active_matches;

            //! Callback function to be called whenever messages get received, takes std::vector of messages as argument, is void
            std::function<void(std::vector<typename MessageType::type>&)> registered_callback;

        } listener_;


        /**
         * \brief Returns qos for the settings s.t. the constructor becomes more readable
         * \param is_reliable Set receiving to best effort (false) / reliable (true)
         * \param is_transient_local Set receiving to transient local
         * \param history_keep_all Set receiving to keep all messages (true) / only the last one (false)
         */
        eprosima::fastdds::dds::DataReaderQos get_qos(bool is_reliable, bool is_transient_local, bool history_keep_all)
        {
            eprosima::fastdds::dds::DataReaderQos data_reader_qos;

            //Initialize reader
            if (is_transient_local)
            {
                auto reliable_policy = eprosima::fastdds::dds::ReliabilityQosPolicy();
                reliable_policy.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;

                auto durability_policy = eprosima::fastdds::dds::DurabilityQosPolicy();
                durability_policy.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;

                data_reader_qos.reliability(reliable_policy);
                data_reader_qos.durability(durability_policy);
            }
            else if (is_reliable)
            {
                auto reliable_policy = eprosima::fastdds::dds::ReliabilityQosPolicy();
                reliable_policy.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
                data_reader_qos.reliability(reliable_policy);
            }
            else
            {
                auto policy = eprosima::fastdds::dds::ReliabilityQosPolicy();
                policy.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;

                data_reader_qos.reliability(policy);
            }

            if(history_keep_all){
              auto policy = eprosima::fastdds::dds::HistoryQosPolicy();
              policy.kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;

              data_reader_qos.history(policy);
            }

            return data_reader_qos;
        }
    public:
        /**
         * \brief Constructor for the AsynReader. This constructor is simpler and creates subscriber, topic etc on the cpm domain participant
         * The reader always uses History::KeepAll
         * \param on_read_callback Callback function that is called by the reader if new data is available. Samples are passed to the function to be processed further.
         * \param participant Domain participant to specify in which domain the reader should operate
         * \param topic_name The name of the topic that is supposed to be used by the reader
         * \param is_reliable If true, the used reader is set to be reliable, else best effort is expected
         * \param is_transient_local If true, the used reader is set to be transient local - in this case, it is also set to reliable
         * \param history_keep_all If true, the internal DDS Reader tries to keep all messages, else only the latest message is kept
         */
        ReaderParent(
            std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
            std::shared_ptr<eprosima::fastdds::dds::DomainParticipant>,
            std::string topic_name, 
            bool is_reliable = false,
            bool is_transient_local = false,
            bool history_keep_all = true
        );

        /**
         * \brief Returns # of matched writers
         */
        size_t matched_publications_size();

        std::shared_ptr<eprosima::fastdds::dds::DataReader> get_reader(){
          return reader;
        }
    };

    template<class MessageType> 
    ReaderParent<MessageType>::ReaderParent(
        std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
        std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant,
        std::string topic_name, 
        bool is_reliable,
        bool is_transient_local,
        bool history_keep_all
    )
    : type_support(new MessageType()), participant_(participant), topic_name(topic_name), listener_(on_read_callback)
    {
        sub = std::shared_ptr<eprosima::fastdds::dds::Subscriber>(
            participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT),
            [&] (eprosima::fastdds::dds::Subscriber* sub)
            {
                if (sub != nullptr)
                {
                    participant_->delete_subscriber(sub);
                }
            }
        );
        assert(sub);

        // Check if Type is already registered, create type
        auto find_type_ret = participant_->find_type(topic_data_type.getName());
        std::cout << "Checking if type exists: " << topic_data_type.getName() << std::endl;
        if(find_type_ret.empty()){
            std::cout << "Type does not exist, creating type" << std::endl;
            auto ret = type_support.register_type(participant_.get());
            assert(ret == eprosima::fastdds::dds::TypeSupport::ReturnCode_t::RETCODE_OK);
        }

        assert(participant_->find_type(topic_data_type.getName()).empty() == false);
        std::cout << "Double check: " << participant_->find_type(topic_data_type.getName()).get_type_name() << std::endl;

        // Create Topic
        auto find_topic = participant_->lookup_topicdescription(topic_name);
        if(find_topic == nullptr){
            std::string type_name_str = topic_data_type.getName();
            topic = std::shared_ptr<eprosima::fastdds::dds::Topic>(
                participant_->create_topic(topic_name, type_name_str, eprosima::fastdds::dds::TOPIC_QOS_DEFAULT),
                [&](eprosima::fastdds::dds::Topic* topic) {
                    if (topic != nullptr)
                    {
                        participant_->delete_topic(topic);
                    }
                }
            );
        }else{
            topic = std::shared_ptr<eprosima::fastdds::dds::Topic>(
                (eprosima::fastdds::dds::Topic*)find_topic,
                [&](eprosima::fastdds::dds::Topic* topic) {
                    if (topic != nullptr)
                    {
                        participant_->delete_topic(topic);
                    }
                }
            );
        }
    
        assert(topic);

        // Create Reader
        std::cout << "Creating ReaderParent" << std::endl;
        reader = std::shared_ptr<eprosima::fastdds::dds::DataReader>(
            sub->create_datareader(topic.get(), get_qos(is_reliable, is_transient_local, history_keep_all), &listener_),
            [&](eprosima::fastdds::dds::DataReader* reader) {
                if (sub != nullptr)
                {
                    sub->delete_datareader(reader);
                }
            }
        );
        assert(reader);      
    }

    template<class MessageType> 
    size_t ReaderParent<MessageType>::matched_publications_size()
    {
        return listener_.active_matches.load();
    }
}