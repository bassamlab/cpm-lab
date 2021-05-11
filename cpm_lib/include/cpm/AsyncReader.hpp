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
#include "cpm/ReaderParent.hpp"

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
    class AsyncReader : public ReaderParent<MessageType>
    {
    public:
        /**
         * \brief Constructor for the AsynReader. This constructor is simpler and creates subscriber, topic etc on the cpm domain participant
         * The reader always uses History::KeepAll
         * \param on_read_callback Callback function that is called by the reader if new data is available. Samples are passed to the function to be processed further.
         * \param topic_name The name of the topic that is supposed to be used by the reader
         * \param is_reliable If true, the used reader is set to be reliable, else best effort is expected
         * \param is_transient_local If true, the used reader is set to be transient local - in this case, it is also set to reliable and keep all
         */
        AsyncReader(
            std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
            std::string topic_name, 
            bool is_reliable = false,
            bool is_transient_local = false
        );

        /**
         * \brief Constructor for the AsynReader. This constructor is simpler and creates subscriber, topic etc on the cpm domain participant
         * The reader always uses History::KeepAll
         * \param on_read_callback Callback function that is called by the reader if new data is available. Samples are passed to the function to be processed further.
         * \param participant Domain participant to specify in which domain the reader should operate
         * \param topic_name The name of the topic that is supposed to be used by the reader
         * \param is_reliable If true, the used reader is set to be reliable, else best effort is expected
         * \param is_transient_local If true, the used reader is set to be transient local - in this case, it is also set to reliable and keep all
         */
        AsyncReader(
            std::function<void(std::vector<typename MessageType::type>&)> on_read_callback,
            cpm::Participant& participant, 
            std::string topic_name, 
            bool is_reliable = false,
            bool is_transient_local = false
        );

        AsyncReader(
            std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
            std::shared_ptr<eprosima::fastdds::dds::DomainParticipant>,
            std::string topic_name, 
            bool is_reliable = false,
            bool is_transient_local = false,
            bool history_keep_all = true
        );
    };

    template<class MessageType> 
    AsyncReader<MessageType>::AsyncReader(
        std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
        std::string topic_name, 
        bool is_reliable,
        bool is_transient_local
    ) : AsyncReader(on_read_callback, cpm::ParticipantSingleton::Instance(), topic_name, is_reliable, is_transient_local, false)
    {}

    template<class MessageType> 
    AsyncReader<MessageType>::AsyncReader(
        std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
        cpm::Participant& participant,
        std::string topic_name, 
        bool is_reliable,
        bool is_transient_local
    ) :  AsyncReader(on_read_callback, participant.get_participant(), topic_name, is_reliable, is_transient_local, false) {}

    template<class MessageType> 
    AsyncReader<MessageType>::AsyncReader(
        std::function<void(std::vector<typename MessageType::type>&)> on_read_callback, 
        std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant,
        std::string topic_name, 
        bool is_reliable,
        bool is_transient_local,
        bool history_keep_all
    )
    : ReaderParent<MessageType>(on_read_callback, participant, topic_name, is_reliable, is_transient_local, history_keep_all)
    {
      
    }
}