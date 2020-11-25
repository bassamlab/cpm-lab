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

#include <dds/pub/DataWriter.hpp>


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
        
        eprosima::fastdds::dds::Publisher* publisher;
        eprosima::fastdds::dds::Topic* topic;
        eprosima::fastdds::dds::DataWriter* writer;

        //dds::pub::DataWriter<typename T::type> dds_writer;

        /**
         * \brief Returns qos for the settings s.t. the constructor becomes more readable
         */
        dds::pub::qos::DataWriterQos get_qos(bool is_reliable, bool history_keep_all, bool is_transient_local)
        {
            auto qos = dds::pub::qos::DataWriterQos();


            if (is_reliable)
            {
                qos.reliability(dds::core::policy::Reliability::Reliable());
            }
            else
            {
                //Already implicitly given
                qos.reliability(dds::core::policy::Reliability::BestEffort());
            }

            if (history_keep_all)
            {
                qos.history(dds::core::policy::History::KeepAll());
            }

            if (is_transient_local)
            {
                qos.durability(dds::core::policy::Durability::TransientLocal());
            }

            return qos;
        }


    public:
        Writer(const Writer&) = delete;
        Writer& operator=(const Writer&) = delete;
        Writer(const Writer&&) = delete;
        Writer& operator=(const Writer&&) = delete;
        
        /**
         * \brief Constructor for a writer which is communicating within the ParticipantSingleton
         * Allows to set the topic name and some QoS settings
         * \param topic Name of the topic to write in
         * \param reliable Set the writer to be reliable (true) or use best effort (false, default)
         * \param history_keep_all Keep all received messages (true) or not (false, default) (relevant for the reader)
         * \param transient_local Resent messages sent before a new participant joined to that participant (true) or not (false, default)
         */
        Writer(std::string topic_name, bool reliable = false, bool history_keep_all = false, bool transient_local = false)
        { 
          Writer(ParticipantSingleton::Instance(), topic_name, reliable, history_keep_all, transient_local);
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
            dds::domain::DomainParticipant & _participant, 
            std::string topic_name, 
            bool reliable = false, 
            bool history_keep_all = false, 
            bool transient_local = false
        )
        {
          
          eprosima::fastdds::dds::DomainParticipant& p_impl = dynamic_cast<eprosima::fastdds::dds::DomainParticipant&>(_participant);

          // Check if Type is already registered
          auto find_type_ret = p_impl.find_type(topic_data_type.getName());
          if(find_type_ret.empty()){
            p_impl.register_type(eprosima::fastdds::dds::TypeSupport(&topic_data_type));
          }

          // Create Topic
          auto find_topic = p_impl.lookup_topicdescription(topic_name);
          if(find_topic == nullptr){
            topic = p_impl.create_topic(topic_name, std::string(topic_data_type.getName()), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
          }
          assert(topic != nullptr);

          publisher = p_impl.create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);
          writer = publisher->create_datawriter(topic, get_qos(reliable, history_keep_all, transient_local));

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

        /**
         * \brief Returns # of matched writers, needs template parameter for topic type
         */
        size_t matched_subscriptions_size()
        {
            return 0;// writer->get_listener().listener.n_matched;
        }
    };
}