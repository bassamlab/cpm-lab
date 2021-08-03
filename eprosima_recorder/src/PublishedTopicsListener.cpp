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

/**
 * \file PublishedTopicsListener.cpp
 * \brief Counterpart to the .hpp
 */

#include "PublishedTopicsListener.hpp"

PublishedTopicsListener::PublishedTopicsListener(
    int domain_id, 
    std::function<void(std::string, std::string, eprosima::fastdds::dds::WriterQos, eprosima::fastdds::dds::DomainParticipant*)> on_publisher_discovered
) 
{
    // Create the participant QoS and configure values
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    std::string identifier = "EprosimaRecorder" + std::to_string(domain_id);
    pqos.name(identifier);
    
    // Create a custom user DomainParticipantListener, register the callback function
    participant_listener = std::make_shared<DiscoveryDomainParticipantListener>(on_publisher_discovered);

    // Pass the listener on DomainParticipant creation.
    participant = std::shared_ptr<eprosima::fastdds::dds::DomainParticipant>(
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
            domain_id, 
            pqos, 
            participant_listener.get()
        ),
        [] (eprosima::fastdds::dds::DomainParticipant* participant) {
            if (participant != nullptr)
            {
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
            }
        }
    );
}

PublishedTopicsListener::DiscoveryDomainParticipantListener::DiscoveryDomainParticipantListener(
    std::function<void(std::string, std::string, eprosima::fastdds::dds::WriterQos, eprosima::fastdds::dds::DomainParticipant*)> _on_publisher_discovered
) :
    on_publisher_discovered(_on_publisher_discovered)
{

}

/* Custom Callback on_participant_discovery */
void PublishedTopicsListener::DiscoveryDomainParticipantListener::on_participant_discovery(
        eprosima::fastdds::dds::DomainParticipant* participant,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    (void)participant;
    switch (info.status){
        case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT:
            /* Process the case when a new DomainParticipant was found in the domain */
            std::cout << "New DomainParticipant '" << info.info.m_participantName <<
                "' with ID '" << info.info.m_guid.entityId << "' and GuidPrefix '" <<
                info.info.m_guid.guidPrefix << "' discovered." << std::endl;
            break;
        case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT:
            /* Process the case when a DomainParticipant changed its QOS */
            break;
        case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
            /* Process the case when a DomainParticipant was removed from the domain */
            std::cout << "New DomainParticipant '" << info.info.m_participantName <<
                "' with ID '" << info.info.m_guid.entityId << "' and GuidPrefix '" <<
                info.info.m_guid.guidPrefix << "' left the domain." << std::endl;
            break;
    }
}

/* Custom Callback on_publisher_discovery */
void PublishedTopicsListener::DiscoveryDomainParticipantListener::on_publisher_discovery(
        eprosima::fastdds::dds::DomainParticipant* participant,
        eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info)
{
    (void)participant;

    switch (info.status){
        case eprosima::fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER:
            /* Process the case when a new publisher was found in the domain */
            std::cout << "New DataWriter publishing under topic '" << info.info.topicName() <<
                "' of type '" << info.info.typeName() << "' discovered" << std::endl;
            
            // Call the callback function to allow for DataReader creation given this new information
            on_publisher_discovered(info.info.typeName().c_str(), info.info.topicName().c_str(), info.info.m_qos, participant);
            break;
        case eprosima::fastrtps::rtps::WriterDiscoveryInfo::CHANGED_QOS_WRITER:
            /* Process the case when a publisher changed its QOS */
            break;
        case eprosima::fastrtps::rtps::WriterDiscoveryInfo::REMOVED_WRITER:
            /* Process the case when a publisher was removed from the domain */
            std::cout << "New DataWriter publishing under topic '" << info.info.topicName() <<
                "' of type '" << info.info.typeName() << "' left the domain." << std::endl;
            break;
    }
}
