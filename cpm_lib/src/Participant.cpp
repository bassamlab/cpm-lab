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

#include "cpm/Participant.hpp"

/**
 * \file Participant.cpp
 * \ingroup cpmlib
 */

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include "cpm/Participant.hpp"
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

#include <sstream>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>

#include <fastrtps/utils/IPLocator.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
namespace cpm
{

    Participant::Participant(int domain_number, bool use_shared_memory)
    { 
        //Create new QoS
        eprosima::fastdds::dds::DomainParticipantQos qos;

        if (use_shared_memory)
        {
            std::cout << "Creating local communication participant..." << std::endl;

            //Create shared memory descriptor
            auto shm_transport = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
            
            //Try to set to shared memory transport only
            qos.transport().use_builtin_transports = false;
            qos.transport().user_transports.push_back(shm_transport);
        }
        else
        {
            //Else use default QoS
            qos = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->get_default_participant_qos();
        }

        participant = std::shared_ptr<eprosima::fastdds::dds::DomainParticipant>(
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domain_number, qos),
            [] (eprosima::fastdds::dds::DomainParticipant* participant) {
                if (participant != nullptr)
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
            }
        );
        assert(participant);
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
        
        participant = std::shared_ptr<eprosima::fastdds::dds::DomainParticipant>(
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domain_number, qos),
            [] (eprosima::fastdds::dds::DomainParticipant* participant) {
                if (participant != nullptr)
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
            }
        );

        if(! participant){
            throw std::invalid_argument("failed to create participant");
        }
    }
    
    /**
     * \brief Constructor for a Server participant 
     * \param domain_number Set the domain ID of the domain within which the communication takes place
     * \param qos_file QoS settings to be imported from an .xml file
     */
    Participant::Participant(int domain_number, discovery_mode server_client, std::string guid, std::string ip, int port)
    {
        DomainParticipantQos pqos;
        switch (server_client)
        {
        case SERVER:
            pqos = create_server_qos(guid, ip, port);
            break;
        case CLIENT:
            pqos = create_client_qos(guid, ip, port);
            break;
        default:
            pqos = PARTICIPANT_QOS_DEFAULT;
            break;
        }

        participant = std::shared_ptr<eprosima::fastdds::dds::DomainParticipant>(
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domain_number, pqos),
            [] (eprosima::fastdds::dds::DomainParticipant* participant) {
                if (participant != nullptr)
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
            }
        );
        

    }
    std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> Participant::get_participant()
    {
        return participant;
    }

    DomainParticipantQos Participant::create_client_qos(std::string guid, std::string ip, uint32_t port)
    {
        // Get default participant QoS
        DomainParticipantQos client_qos = PARTICIPANT_QOS_DEFAULT;

        // Set participant as CLIENT
        client_qos.wire_protocol().builtin.discovery_config.discoveryProtocol =
                DiscoveryProtocol_t::CLIENT;

        // Set SERVER's GUID prefix
        RemoteServerAttributes remote_server_att;
        remote_server_att.ReadguidPrefix(guid.c_str());

        // Set SERVER's listening locator for PDP
        Locator_t locator;
        IPLocator::setIPv4(locator, ip);
        locator.port = port;
        remote_server_att.metatrafficUnicastLocatorList.push_back(locator);

        // Add remote SERVER to CLIENT's list of SERVERs
        client_qos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(remote_server_att);

        // Set ping period to 250 ms
        client_qos.wire_protocol().builtin.discovery_config.discoveryServer_client_syncperiod =
               eprosima::fastrtps::Duration_t(0, 250000000);

        return client_qos;
    }

    DomainParticipantQos Participant::create_server_qos(std::string guid, std::string ip, uint32_t port)
    {
        DomainParticipantQos server_qos = PARTICIPANT_QOS_DEFAULT;

        // Set participant as SERVER
        server_qos.wire_protocol().builtin.discovery_config.discoveryProtocol =
                DiscoveryProtocol_t::SERVER;

        // Set SERVER's GUID prefix
        std::istringstream(guid) >> server_qos.wire_protocol().prefix;

        // Set SERVER's listening locator for PDP
        Locator_t locator;
        IPLocator::setIPv4(locator, ip);
        locator.port = port;
        server_qos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);

        return server_qos;
    }
}