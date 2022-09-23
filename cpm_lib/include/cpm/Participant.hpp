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
        enum DiscoveryMode {SIMPLE, SERVER, CLIENT};

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
         * \brief Constructor for a Server/Client participant 
         * \param domain_number Set the domain ID of the domain within which the communication takes place
         * \param discovery_mode The type of participant. 
         *  Use DiscoveryMode::CLIENT or DiscoveryMode::SERVER for Discovery Server mode. 
         *  Use DiscoveryMode::SIMPLE for simple endpoint discovery.
         * \param discovery_server_id 
         * \param discovery_server_ip
         * \param discovery_server_port
         */
        Participant(int domain_number, DiscoveryMode discovery_mode, std::string discovery_server_id, std::string discovery_server_ip, int discovery_server_port);
    
        /**
         * \brief Returns a shared_ptr to the encapsulated eProsima domain participant
         */
        std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> get_participant();

        /**
         * \brief Creates QOS settings that are needed for a client Participant when using client the discovery server mode
         */
        static eprosima::fastdds::dds::DomainParticipantQos create_client_qos(std::string guid, std::string ip, uint32_t port);

        /**
         * \brief Creates QOS settings that are needed for a client Participant when using client the discovery server mode
         */
        static eprosima::fastdds::dds::DomainParticipantQos create_server_qos(std::string guid, std::string ip, uint32_t port);

        /**
         * \brief Creates QOS settings for preallocation. Useful for RT behaviour.
         */
        static eprosima::fastdds::dds::DomainParticipantQos create_preallocation_qos(eprosima::fastdds::dds::DomainParticipantQos participant_qos, size_t participants, size_t readers, size_t writers);
    };
}
