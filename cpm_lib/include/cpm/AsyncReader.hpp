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

#include <dds/sub/Subscriber.hpp>
#include <dds/sub/DataReader.hpp>

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/Participant.hpp"

/**
 * \file AsyncReader.hpp
 */

namespace cpm 
{
    /**
     * \class AsyncReader
     * \brief This class is a wrapper for a data reader that uses an AsyncWaitSet to call a callback function whenever any new data is available
     * Template: Class of the message objects, depending on which IDL file is used
     * \ingroup cpmlib
     */ 
    template<class MessageType> 
    class AsyncReader : public eprosima::fastdds::dds::DataReaderListener
    {
    private:
        //Reader and waitset for receiving data and calling the callback function
        eprosima::fastdds::dds::Subscriber* sub;
        eprosima::fastdds::dds::DataReader* reader;
        eprosima::fastdds::dds::Topic* topic;
        eprosima::fastdds::dds::TypeSupport type_support;
        MessageType topic_data_type;

        std::vector<typename MessageType::type> buffer;

        std::function<void(std::vector<typename MessageType::type>&)> target; 


        void on_data_available(
          eprosima::fastdds::dds::DataReader* reader) override {

          buffer.clear();              

          eprosima::fastdds::dds::SampleInfo info;
          typename MessageType::type data;
          while(reader->take_next_sample(&data, &info) == ReturnCode_t::RETCODE_OK)
          {
              if (info.instance_state == eprosima::fastdds::dds::ALIVE)
              {
                  buffer.push_back(data);
              }
          }

          if(buffer.size() > 0){
            target(buffer);
          }
          }

      void on_subscription_matched(
          eprosima::fastdds::dds::DataReader* reader,
          const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override{
      }


        /**
         * \brief Returns qos for the settings s.t. the constructor becomes more readable
         * \param is_reliable If the QoS for DDS messages should be set to reliable (true) or best effort (false) messaging
         * \param is_transient_local If true, and if the Writer is still present, the Reader receives data that was sent before it went online
         */
        dds::sub::qos::DataReaderQos get_qos(bool is_reliable, bool is_transient_local, bool history_keep_all)
        {
            dds::sub::qos::DataReaderQos data_reader_qos;
            //Initialize reader
            if (is_transient_local)
            {
                data_reader_qos.reliability(dds::core::policy::Reliability::Reliable());
                data_reader_qos.durability(dds::core::policy::Durability::TransientLocal());
            }
            else if (is_reliable)
            {
                data_reader_qos.reliability(dds::core::policy::Reliability::Reliable());
            }
            else
            {
                data_reader_qos.reliability(dds::core::policy::Reliability::BestEffort());
            }

            if(history_keep_all){
              data_reader_qos.history(dds::core::policy::History::KeepAll());
            }

            return data_reader_qos;
        }

        /**
         * \brief Handler that takes unread samples, releases the waitset and calls the callback function provided by the user
         * \param func The callback function provided by the user
         */
        //void handler(std::function<void(dds::sub::LoanedSamples<MessageType>&)> func);

        /**
         * \brief Handler that takes unread samples, copies them to a vector, releases the waitset and calls the callback function provided by the user
         * \param func The callback function provided by the user
         */
        //void handler_vec(std::function<void(std::vector<MessageType>&)> func);
    public:
        /**
         * \brief Constructor for the AsynReader. This constructor is simpler and creates subscriber, topic etc on the cpm domain participant
         * The reader always uses History::KeepAll
         * \param func Callback function that is called by the reader if new data is available. Samples are passed to the function to be processed further.
         * \param topic_name The name of the topic that is supposed to be used by the reader
         * \param is_reliable If true, the used reader is set to be reliable, else best effort is expected
         * \param is_transient_local If true, the used reader is set to be transient local - in this case, it is also set to reliable and keep all
         */
        AsyncReader(
            std::function<void(std::vector<typename MessageType::type>&)> func, 
            std::string topic_name, 
            bool is_reliable = false,
            bool is_transient_local = false,
            eprosima::fastdds::dds::DataReaderListener* custom_listener = nullptr
        );

        /**
         * \brief Constructor for the AsynReader. This constructor is simpler and creates subscriber, topic etc on the cpm domain participant
         * The reader always uses History::KeepAll
         * \param func Callback function that is called by the reader if new data is available. Samples are passed to the function to be processed further.
         * \param participant Domain participant to specify in which domain the reader should operate
         * \param topic_name The name of the topic that is supposed to be used by the reader
         * \param is_reliable If true, the used reader is set to be reliable, else best effort is expected
         * \param is_transient_local If true, the used reader is set to be transient local - in this case, it is also set to reliable and keep all
         */
        AsyncReader(
            std::function<void(std::vector<typename MessageType::type>&)> func,
            cpm::Participant& participant, 
            std::string topic_name, 
            bool is_reliable = false,
            bool is_transient_local = false,
            eprosima::fastdds::dds::DataReaderListener* custom_listener = nullptr
        );

        AsyncReader(
            std::function<void(std::vector<typename MessageType::type>&)> func, 
            dds::domain::DomainParticipant&,
            std::string topic_name, 
            bool is_reliable,
            bool is_transient_local,
            bool history_keep_all = true,
            eprosima::fastdds::dds::DataReaderListener* custom_listener = nullptr
        );

        /**
         * \brief Returns # of matched writers
         */
        size_t matched_publications_size();

        eprosima::fastdds::dds::DataReader* get_reader(){
          return reader;
        }
    };

    template<class MessageType> 
    AsyncReader<MessageType>::AsyncReader(
        std::function<void(std::vector<typename MessageType::type>&)> func, 
        std::string topic_name, 
        bool is_reliable,
        bool is_transient_local,
        eprosima::fastdds::dds::DataReaderListener* custom_listener
    )
    {
      dds::domain::DomainParticipant& p = cpm::ParticipantSingleton::Instance();
      AsyncReader(func, p, topic_name, is_reliable, is_transient_local);
    }

    template<class MessageType> 
    AsyncReader<MessageType>::AsyncReader(
        std::function<void(std::vector<typename MessageType::type>&)> func, 
        cpm::Participant& participant,
        std::string topic_name, 
        bool is_reliable,
        bool is_transient_local,
        eprosima::fastdds::dds::DataReaderListener* custom_listener
    ){
      AsyncReader(func, participant.get_participant(), topic_name, is_reliable, is_transient_local);
    }

    template<class MessageType> 
    AsyncReader<MessageType>::AsyncReader(
        std::function<void(std::vector<typename MessageType::type>&)> func, 
        dds::domain::DomainParticipant& participant,
        std::string topic_name, 
        bool is_reliable,
        bool is_transient_local,
        bool history_keep_all,
        eprosima::fastdds::dds::DataReaderListener* custom_listener
    )
    :sub(), target(func)
    {

      buffer.reserve(100);

      eprosima::fastdds::dds::DomainParticipant& p_impl = dynamic_cast<eprosima::fastdds::dds::DomainParticipant&>(participant);
      sub = p_impl.create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
      assert(sub != nullptr);


      // Register Type      
      // Check if Type Name already registered
      auto find_type_ret = p_impl.find_type(topic_data_type.getName());
      if(find_type_ret.empty()){
        p_impl.register_type(eprosima::fastdds::dds::TypeSupport(&topic_data_type));
      }

      // Create Topic
      auto find_topic = p_impl.lookup_topicdescription(topic_name);
      if(find_topic == nullptr){
        topic = p_impl.create_topic(topic_name,topic_data_type.getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
      }

      assert(topic != nullptr);

      // Create Reader
      if(custom_listener == nullptr){
        reader = sub->create_datareader(topic, get_qos(is_reliable, is_transient_local, history_keep_all), this);
      }else{
        reader = sub->create_datareader(topic, get_qos(is_reliable, is_transient_local, history_keep_all), custom_listener);
      }
      assert(reader != nullptr);      
    }

    template<class MessageType> 
    size_t AsyncReader<MessageType>::matched_publications_size()
    {
        //auto matched_pub = dds::sub::matched_publications(reader);
        //return matched_pub.size();
        return 0;
    }
}