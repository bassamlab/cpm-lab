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

#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>

#include <memory>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/get_topic.hpp"

#include <dds/core/QosProvider.hpp>
#include <dds/dds.hpp>
#include <dds/core/ddscore.hpp>

// #include <experimental/filesystem>

namespace cpm
{
    /**
     * \class Writer
     * \brief Creates a DDS Writer that can be used for writing / publishing messages
     * This encapsulation allows for changes e.g. in the participant or QoS without 
     * the need to change the implementation across the whole project
     * \ingroup cpmlib
     */
    template<class T>
    class Writer
    {
    private:
        T topic_data_type;
        
        eprosima::fastdds::dds::TypeSupport type_support;
        std::shared_ptr<eprosima::fastdds::dds::Publisher> publisher;
        std::shared_ptr<eprosima::fastdds::dds::Topic> topic;
        std::shared_ptr<eprosima::fastdds::dds::DataWriter> writer;
        std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant_;

        class PubListener : public eprosima::fastdds::dds::DataWriterListener
        {
        public:

            PubListener()
                : matched_(0)
            {
            }

            ~PubListener() override
            {
            }

            void on_publication_matched(
                    eprosima::fastdds::dds::DataWriter*,
                    const eprosima::fastdds::dds::PublicationMatchedStatus& info) override
            {
                matched_ = info.total_count;
            }

            std::atomic_int matched_;

        } listener_;

        //dds::pub::DataWriter<typename T::type> dds_writer;

        Writer(const Writer&) = delete;
        Writer& operator=(const Writer&) = delete;
        Writer(const Writer&&) = delete;
        Writer& operator=(const Writer&&) = delete;

        /**
         * \brief Returns qos for the settings s.t. the constructor becomes more readable
         */
        eprosima::fastdds::dds::DataWriterQos get_qos(bool is_reliable, bool history_keep_all, bool is_transient_local)
        {
            auto qos = eprosima::fastdds::dds::DataWriterQos();


            if (is_reliable)
            {
                auto policy = eprosima::fastdds::dds::ReliabilityQosPolicy();
                policy.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
                qos.reliability(policy);
            }
            else
            {
                //Already implicitly given
                auto policy = eprosima::fastdds::dds::ReliabilityQosPolicy();
                policy.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
                qos.reliability(policy);
            }

            if (history_keep_all)
            {
                auto policy = eprosima::fastdds::dds::HistoryQosPolicy();
                policy.kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
                qos.history(policy);
            }else{
                auto policy = eprosima::fastdds::dds::HistoryQosPolicy();
                policy.kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
                qos.history(policy);  
            }

            if (is_transient_local)
            {
                auto policy = eprosima::fastdds::dds::DurabilityQosPolicy();
                policy.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
                qos.durability(policy);
            }else{
                auto policy = eprosima::fastdds::dds::DurabilityQosPolicy();
                policy.kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
                qos.durability(policy);
            }

            return qos;
        }


    public:

        /**
         * \brief Destructor to clear eProsima data
         * Virtual makes sure that for derived classes this destructor still gets called
         */
        virtual ~Writer(){
            std::cout << "Deleting cpm::Writer" << std::endl;
        }

        /**
         * \brief Constructor for a writer which is communicating within the ParticipantSingleton
         * Allows to set the topic name and some QoS settings
         * \param topic Name of the topic to write in
         * \param reliable Set the writer to be reliable (true) or use best effort (false, default)
         * \param history_keep_all Keep all received messages (true) or not (false, default) (relevant for the reader)
         * \param transient_local Resent messages sent before a new participant joined to that participant (true) or not (false, default)
         */
        Writer(std::string topic_name, bool reliable = false, bool history_keep_all = false, bool transient_local = false)
        : Writer(ParticipantSingleton::Instance(), topic_name, reliable, history_keep_all, transient_local)
        {       
        }

        /**
         * \brief Constructor for a writer which is communicating within the ParticipantSingleton
         * Allows to set the topic name and some QoS settings
         * \param topic Name of the topic to write in
         * \param qos_xml_path Path for setting additional QoS
         * \param library The loaded library to use
         */
        Writer(std::string topic, std::string qos_xml_path, std::string library)
        :dds_writer(dds::pub::Publisher(ParticipantSingleton::Instance()), cpm::get_topic<T>(topic), dds::core::QosProvider(qos_xml_path, library).datawriter_qos())
        { 
        
        }

        /**
         * \brief Constructor for a writer that communicates within another domain
         * Allows to set the topic name and some QoS settings
         * \param _participant The domain (participant) in which to write
         * \param topic Name of the topic to write in
         * \param reliable Set the writer to be reliable (true) or use best effort (false, default)
         * \param history_keep_all Keep all received messages (true) or not (false, default) (relevant for the reader)
         * \param transient_local Resent messages sent before a new participant joined to that participant (true) or not (false, default)
         */
        Writer(
            std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> _participant, 
            std::string topic_name, 
            bool reliable = false, 
            bool history_keep_all = false, 
            bool transient_local = false
        ) : type_support(new T()), participant_(_participant)
        {
            std::cout << "Creating Writer " << topic_name << " : " << topic_data_type.getName() << std::endl;

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
                    [participant_ = participant_](eprosima::fastdds::dds::Topic* topic) {
                        if (topic != nullptr)
                        {
                            std::cout << "PDL A" << std::endl;
                            participant_->delete_topic(topic);
                            std::cout << "PDL A end" << std::endl;
                        }
                    }
                );
            }else{
                topic = std::shared_ptr<eprosima::fastdds::dds::Topic>(
                    (eprosima::fastdds::dds::Topic*)find_topic,
                    [participant_ = participant_](eprosima::fastdds::dds::Topic* topic) {
                        if (topic != nullptr)
                        {
                            std::cout << "PDL B" << std::endl;
                            participant_->delete_topic(topic);
                            std::cout << "PDL B end" << std::endl;
                        }
                    }
                );
            }

            std::cout << "Double check topic " <<  topic->get_type_name() << " " << topic->get_name() << std::endl;
            assert(topic);
            assert(participant_->find_type(topic->get_type_name()).empty() == false);

            //Create Publisher
            publisher = std::shared_ptr<eprosima::fastdds::dds::Publisher>(
                participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT),
                [participant_ = participant_](eprosima::fastdds::dds::Publisher* publisher) {
                    if (publisher != nullptr)
                    {
                        std::cout << "PDL C" << std::endl;
                        participant_->delete_publisher(publisher);
                        std::cout << "PDL C end" << std::endl;
                    }
                }
            );

            //Create Writer
            writer = std::shared_ptr<eprosima::fastdds::dds::DataWriter>(
                publisher->create_datawriter(topic.get(), get_qos(reliable, history_keep_all, transient_local), &listener_),
                [publisher = publisher](eprosima::fastdds::dds::DataWriter* writer) {
                    if (writer != nullptr)
                    {
                        std::cout << "PDL d" << std::endl;
                        publisher->delete_datawriter(writer);
                        std::cout << "PDL d end" << std::endl;
                    }
                }
            );

            assert(writer);                      
        }
        
        /**
         * \brief Send a message in the DDS network using the writer
         * \param msg The message to send
         */
        void write(typename T::type& msg)
        {
            //DDS operations are assumed to be thread safe, so don't use a mutex here
            writer->write(&msg);
        }

        /**<
         * \brief Returns # of matched writers, needs template parameter for topic type
         */
        size_t matched_subscriptions_size()
        {
            return static_cast<size_t>(listener_.matched_.load());
        }
    };
}