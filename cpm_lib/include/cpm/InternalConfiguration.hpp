#pragma once
#include <string>

namespace cpm
{
    /**
     * \class InternalConfiguration
     * \brief This class sets up the DDS domain, logging ID and DDS initial peer, and its init function 
     * should be used whenever the cpm library is used, at the start of the program
     * \ingroup cpmlib
     */
    class InternalConfiguration
    {
    private: 
    
        //! The actual Singleton instance
        static InternalConfiguration the_instance;

        //! DDS domain for the participant singleton, so for most of the communication
        int dds_domain = 0;
        //! ID for log messages, usually program type, e.g. "LCC" or "middleware"
        std::string logging_id = "uninitialized";
        //! Wether this instance provides a discovery server or is a discovery client
        std::string client_server;
        //! Discovery server id, has to be unique in the DDS network
        std::string discovery_server_id;
        //! Discovery server ip 
        std::string discovery_server_ip;
        //! //! Discovery server port
        int discovery_server_port;
       

        /**
         * \brief Empty default constructor, private, can / should not be used
         */
        InternalConfiguration(){}

        /**
         * \brief Constructor, private. Used internally by the Instance() function to create the singleton.
         * \param _dds_domain DDS Domain for the Participant Singleton
         * \param _logging_id Logging ID for the Logger
         * \param _dds_initial_peer Set initial peer(s) for the DDS communication
         */
        InternalConfiguration(
            int _dds_domain,
            std::string _logging_id,
            std::string _client_server,
            std::string _discovery_server_id,
            std::string _discovery_server_ip,
            int _discovery_server_port
        )
        :dds_domain(_dds_domain)
        ,logging_id(_logging_id)
        ,client_server(_client_server)
        ,discovery_server_id(_discovery_server_id)
        ,discovery_server_ip(_discovery_server_ip)
        ,discovery_server_port(_discovery_server_port)
        {}

    public:

        /**
         * \brief Get the set DDS Domain
         */
        int get_dds_domain() { return dds_domain; }

        /**
         * \brief Get the set logging ID
         */
        std::string get_logging_id() { return logging_id; }

        /**
         * \brief Get the status of `client_server
         */
        std::string get_client_server() { return client_server; }

        /**
         * \brief Get the the discovery server id
         */
        std::string get_discovery_server_id() { return discovery_server_id; }

        /**
         * \brief Get the the discovery server locator (IP)
         */
        std::string get_discovery_server_ip() { return discovery_server_ip; }

        /**
         * \brief Get the the discovery server port
         */
        int get_discovery_server_port() { return discovery_server_port; }
        
        /**
         * \brief Init function that should be called at the start of every program that uses the cpm lib
         * Initializes the Singleton and values used by other parts of the library, which are read from the command line:
         * --dds_domain
         * --dds_initial_peer
         * --logging_id
         * --client_server ("client" or "server" or empty if not used)
         * --discovery_server_id
         * --discovery_server_ip
         * --get_discovery_server_port
         */
        static void init(int argc, char *argv[]);

        /**
         * \brief Get access to the internal configuration Singleton, should mostly / only be used internally for constructing other parts of the library
         */
        static InternalConfiguration& Instance() {return the_instance;}
    };
}