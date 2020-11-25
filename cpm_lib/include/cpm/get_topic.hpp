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



#include <dds/topic/Topic.hpp>
#include <string>
#include <mutex>
#include "cpm/ParticipantSingleton.hpp"

/*
namespace cpm
{
    template<typename T>
    dds::topic::Topic<T> get_topic(const dds::domain::DomainParticipant& participant, std::string topic_name)
    {

        static std::mutex get_topic_mutex;
        std::lock_guard<std::mutex> lock(get_topic_mutex);

        eprosima::fastdds::dds::DomainParticipant& p_impl = dynamic_cast<eprosima::fastdds::dds::DomainParticipant&>(participant);
        auto topic_description = p_impl.lookup_topicdescription(topic_name);
        topic_description->

        try
        {
            dds::topic::Topic<T> topic = dds::topic::find<dds::topic::Topic<T>>(participant, topic_name);
            if (topic == dds::core::null) {
                topic = dds::topic::Topic<T>(participant, topic_name);
            }

            return topic;
        }
        catch(...)
        {
            return dds::topic::Topic<T>(participant, topic_name);
        }
    }
    template<typename T>
    dds::topic::Topic<T> get_topic(std::string topic_name)
    {
        return get_topic<T>(cpm::ParticipantSingleton::Instance(), topic_name);
    }
}

*/