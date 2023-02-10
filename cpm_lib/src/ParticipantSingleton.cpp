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

#include "cpm/ParticipantSingleton.hpp"
#include "cpm/Participant.hpp"
#include "cpm/InternalConfiguration.hpp"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <memory>
#include <random>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

using namespace eprosima::fastdds;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::dds;

/**
 * \file ParticipantSingleton.cpp
 * \ingroup cpmlib
 */
namespace cpm 
{
    eprosima::fastdds::dds::DomainParticipantQos ParticipantSingleton::CreateQos()
    {
        if (cpm::InternalConfiguration::Instance().is_valid_discovery_server_config() )
        {
            return Participant::create_client_qos(
                cpm::InternalConfiguration::Instance().get_discovery_server_id(),
                cpm::InternalConfiguration::Instance().get_discovery_server_ip(),
                cpm::InternalConfiguration::Instance().get_discovery_server_port());
        }
        DomainParticipantQos participant_qos = eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
        participant_qos.name("ParticipantSingleton");
        return participant_qos;
    }

    std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> ParticipantSingleton::Instance() {
        static std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> instance_(
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
                eprosima::fastdds::dds::DomainId_t(cpm::InternalConfiguration::Instance().get_dds_domain()), 
                ParticipantSingleton::CreateQos()
            ),
            [] (eprosima::fastdds::dds::DomainParticipant* instance_) {
                if (instance_ != nullptr)
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(instance_);
            }
        );
        return instance_;
    }
}