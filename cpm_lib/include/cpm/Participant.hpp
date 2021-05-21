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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

namespace cpm
{
    /**
     * \class Participant
     * \brief Creates a DDS Participant, use this for abstraction
     * Also allows for loading .xml QoS files
     * \ingroup cpmlib
     */
    class Participant
    {
    private:    
        //! Internal DDS participant that is abstracted by this class
        std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant;

    public:
        Participant(const Participant&) = delete;
        Participant& operator=(const Participant&) = delete;
        Participant(const Participant&&) = delete;
        Participant& operator=(const Participant&&) = delete;
        
        /**
         * \brief Constructor for a participant 
         * \param domain_number Set the domain ID of the domain within which the communication takes place
         * \param use_shared_memory If true, the participant can only communicate on the local machine. 
         * This may be interesting in case you do not want its messages to "pollute" the network, and is also faster.
         */
        Participant(int domain_number, bool use_shared_memory = false);

        /**
         * \brief Constructor for a participant 
         * \param domain_number Set the domain ID of the domain within which the communication takes place
         * \param qos_file QoS settings to be imported from an .xml file
         */
        Participant(int domain_number, std::string qos_file);
        
        /**
         * \brief Returns a shared_ptr to the encapsulated eProsima domain participant
         */
        std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> get_participant();
    };
}
