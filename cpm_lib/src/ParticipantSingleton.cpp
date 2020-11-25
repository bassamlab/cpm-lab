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
#include "cpm/InternalConfiguration.hpp"
#include <memory>

/**
 * \file ParticipantSingleton.cpp
 * \ingroup cpmlib
 */
namespace cpm 
{
    std::mutex ParticipantSingleton::creation_mutex;
    
    dds::domain::DomainParticipant& ParticipantSingleton::Instance() {

        static std::shared_ptr<dds::domain::DomainParticipant> myInstance = nullptr;

        //For efficiency reasons: After creation, this check suffices, so only use the mutex if 
        //the object was not yet created
        if(!myInstance)
        {
            dds::domain::qos::DomainParticipantQos domainParticipantQos = dds::domain::DomainParticipant::default_participant_qos();

                myInstance = std::make_shared<dds::domain::DomainParticipant>(
                    cpm::InternalConfiguration::Instance().get_dds_domain(),
                    domainParticipantQos
                );
            }
        }


        return *myInstance;
    }
}