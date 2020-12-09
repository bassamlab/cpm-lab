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

/**
 * \class Participant.hpp
 * \brief Creates a DDS Participant, use this for abstraction
 * Also allows for loading .xml QoS files
 */

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include "cpm/Participant.hpp"
#include <fastrtps/xmlparser/XMLProfileManager.h>

namespace cpm
{

    Participant::Participant(int domain_number)
    { 
        auto qos = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->get_default_participant_qos();
        participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domain_number, qos);
        assert(participant != nullptr);
    }

    /**
     * \brief Constructor for a participant 
     * \param domain_number Set the domain ID of the domain within which the communication takes place
     * \param qos_file QoS settings to be imported from an .xml file
     */
    Participant::Participant(int domain_number, std::string qos_file)
    {
      auto ret_xml = eprosima::fastrtps::xmlparser::XMLProfileManager::loadXMLFile(qos_file);
      if(ret_xml != eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK){
        throw std::invalid_argument("error loading xml profile");
      }

      eprosima::fastdds::dds::DomainParticipantQos qos;
      auto ret_pf = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->get_participant_qos_from_profile("domainparticipant_profile_name", qos);
      if(ret_pf != ReturnCode_t::RETCODE_OK){
        throw std::invalid_argument("unable to create participant from xml profile");
      }
      participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domain_number, qos);
      if(participant == nullptr){
        throw std::invalid_argument("failed to create participant");
      }
    }

    Participant::~Participant(){
      delete participant;
    }
    
    eprosima::fastdds::dds::DomainParticipant& Participant::get_participant()
    {
        return *participant;
    }

}