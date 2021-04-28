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
    class Writer : public eprosima::fastdds::dds::DataWriterListener
    {
    private:
        T topic_data_type;
        
        eprosima::fastdds::dds::TypeSupport type_support;
        eprosima::fastdds::dds::Publisher* publisher = nullptr;
        eprosima::fastdds::dds::Topic* topic = nullptr;
        eprosima::fastdds::dds::DataWriter* writer = nullptr;
        eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;


        //dds::pub::DataWriter<typename T::type> dds_writer;

        Writer(const Writer&) = delete;
        Writer& operator=(const Writer&) = delete;
        Writer(const Writer&&) = delete;
        Writer& operator=(const Writer&&) = delete;

        unsigned int active_matches = 0;

        void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* reader,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override{
              active_matches = info.total_count;
        }

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

        ~Writer(){
          std::cout << "Deleting cpm::Writer" << std::endl;
          publisher->delete_datawriter(writer);
          participant_->delete_publisher(publisher);
          participant_->delete_topic(topic);
          
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
            eprosima::fastdds::dds::DomainParticipant& _participant, 
            std::string topic_name, 
            bool reliable = false, 
            bool history_keep_all = false, 
            bool transient_local = false
        ) : type_support(new T()), participant_(&_participant)
        {
          std::cout << "Creating Writer " << topic_name << " : " << topic_data_type.getName() << std::endl;
          // Check if Type is already registered
          auto find_type_ret = _participant.find_type(topic_data_type.getName());
          std::cout << "Checking if type exists: " << topic_data_type.getName() << std::endl;
          if(find_type_ret.empty()){
            std::cout << "Type does not exist, creating type" << std::endl;
            auto ret = _participant.register_type(type_support);
            assert(ret == ReturnCode_t::RETCODE_OK);
          }

          assert(_participant.find_type(topic_data_type.getName()).empty() == false);
          std::cout << "Double check: " << _participant.find_type(topic_data_type.getName()).get_type_name() << std::endl;

          // Create Topic
          auto find_topic = _participant.lookup_topicdescription(topic_name);
          if(find_topic == nullptr){
            std::string type_name_str = topic_data_type.getName();
            topic = _participant.create_topic(topic_name, type_name_str, eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
          }else{
            topic = (eprosima::fastdds::dds::Topic*)find_topic;
          }

          std::cout << "Double check topic " <<  topic->get_type_name() << " " << topic->get_name() << std::endl;
          assert(topic != nullptr);
          assert(_participant.find_type(topic->get_type_name()).empty() == false);

          publisher = _participant.create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
          writer = publisher->create_datawriter(topic, get_qos(reliable, history_keep_all, transient_local), this);

          assert(writer != nullptr);                      
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
            return active_matches;
        }
    };
}