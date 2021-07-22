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

#include <chrono>
#include <iostream>
#include <thread>
#include <unistd.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

#include "cpm/init.hpp"
#include "cpm/Logging.hpp"

/**
 * \file VisualizationTest.cpp
 * \brief Test scenario: Creates and sends visualization messages that should be visible in the LCC' MapViewUi
 * \ingroup lcc
 */

//! Taken from eProsima website, modified
class DiscoveryDomainParticipantListener : public eprosima::fastdds::dds::DomainParticipantListener
{
public:
    //! Constructor to set up maps to store discovered topics and topic names
    DiscoveryDomainParticipantListener() {
        //TODO pass map, fill it with info in callbacks
    }

private:
    /* Custom Callback on_participant_discovery */
    virtual void on_participant_discovery(
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

    /* Custom Callback on_subscriber_discovery */
    virtual void on_subscriber_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info)
    {
        (void)participant;
        switch (info.status){
            case eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER:
                /* Process the case when a new subscriber was found in the domain */
                std::cout << "New DataReader subscribed to topic '" << info.info.topicName() <<
                    "' of type '" << info.info.typeName() << "' discovered" << std::endl;
                break;
            case eprosima::fastrtps::rtps::ReaderDiscoveryInfo::CHANGED_QOS_READER:
                /* Process the case when a subscriber changed its QOS */
                break;
            case eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER:
                /* Process the case when a subscriber was removed from the domain */
                std::cout << "New DataReader subscribed to topic '" << info.info.topicName() <<
                    "' of type '" << info.info.typeName() << "' left the domain." << std::endl;
                break;
        }
    }

    /* Custom Callback on_publisher_discovery */
    virtual void on_publisher_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info)
    {
        (void)participant;
        switch (info.status){
            case eprosima::fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER:
                /* Process the case when a new publisher was found in the domain */
                std::cout << "New DataWriter publishing under topic '" << info.info.topicName() <<
                    "' of type '" << info.info.typeName() << "' discovered" << std::endl;
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

    /* Custom Callback on_type_discovery */
    virtual void on_type_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastrtps::rtps::SampleIdentity& request_sample_id,
            const eprosima::fastrtps::string_255& topic,
            const eprosima::fastrtps::types::TypeIdentifier* identifier,
            const eprosima::fastrtps::types::TypeObject* object,
            eprosima::fastrtps::types::DynamicType_ptr dyn_type)
    {
        (void)participant, (void)request_sample_id, (void)topic, (void)identifier, (void)object, (void)dyn_type;
        std::cout << "New data type of topic '" << topic << "' discovered." << std::endl;
    }

};

int main(int argc, char *argv[]) {
    cpm::init(argc, argv);
    cpm::Logging::Instance().set_id("Logger_test");

    std::cout << "Creating domain participant listener..." << std::endl;

    // Create the participant QoS and configure values
    eprosima::fastdds::dds::DomainParticipantQos pqos;

    // Create a custom user DomainParticipantListener
    DiscoveryDomainParticipantListener* plistener = new DiscoveryDomainParticipantListener();
    // Pass the listener on DomainParticipant creation.
    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        12, pqos, plistener);

    usleep(5000000);

    //Delete everything
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
    delete plistener;
    plistener = NULL;

    std::cout << "Shutting down..." << std::endl;

    return 0;
}