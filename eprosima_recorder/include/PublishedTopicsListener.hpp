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

#include <chrono>
#include <iostream>
#include <thread>
#include <unistd.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

#include "RecorderManager.hpp"

/**
 * \class PublishedTopicsListener
 * \brief Creates a DDS Domain Participant in the given domain with default QoS settings,
 * then sets up a DomainParticipantListener, listening for new publishers that join
 * the domain. Whenever a new publisher is found, the given callback method is called 
 * and can be used to set up listeners that listen to data writers on these publishers
 * for recording purposes
 */
class PublishedTopicsListener {
public: 
    /**
     * \brief Constructor. Sets up the listener given a domain ID and a callback to call
     * whenever a publisher is discovered.
     * \param domain_id The domain ID within which the discovery of publishers should take place
     * \param _manager Data class that manages all created topic listeners
     */
    PublishedTopicsListener(
        int domain_id, 
        RecorderManager _manager
    );

private:
    /**
     * \brief DiscoveryDomainParticipantListener
     * \brief Nested helper class: Used by the DomainParticipant to react to discovered participants etc.
     */
    class DiscoveryDomainParticipantListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:
        /**
         * \brief Constructor to set up the object and register a callback that gets called whenever a new publisher is discovered
         * \param _manager Data class that manages all created topic listeners
         */
        DiscoveryDomainParticipantListener(
            RecorderManager _manager
        );

    private:
        /**
         * \brief Custom callback for participant discovery, currently not actually used
         * except for log messages
         * \param participant The DomainParticipant that uses this listener
         * \param info Info about the other discovered participant
         */
        virtual void on_participant_discovery(
                eprosima::fastdds::dds::DomainParticipant* participant,
                eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info);

        // Unused, we cannot monitor subscribers (as they do not send messages)
        // virtual void on_subscriber_discovery(
        //         eprosima::fastdds::dds::DomainParticipant* participant,
        //         eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info);

        /**
         * \brief Custom callback for publisher discovery, used to obtain information
         * about the used topic type (string), name (string) and publisher QoS
         * \param participant The DomainParticipant that uses this listener
         * \param info Info about the discovered publisher
         */
        virtual void on_publisher_discovery(
                eprosima::fastdds::dds::DomainParticipant* participant,
                eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info);

        //! Manages all topic recorders
        RecorderManager manager;
    };

    //! Participant listener that is registered with the domain participant to discover publishers in the domain
    std::shared_ptr<DiscoveryDomainParticipantListener> participant_listener;
    //! Domain participant that uses the participant_listener to discover publishers in its domain
    std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant;
};