#include <experimental/filesystem>
#include <functional>
#include <memory>
#include <sstream>
#include <string>

#include "cpm/dds/ReadyStatus.h"

#include "cpm/ParticipantSingleton.hpp"

#include "cpm/TimerFD.hpp"
#include "cpm/Logging.hpp"
#include "cpm/CommandLineReader.hpp"
#include "cpm/init.hpp"

//To get the IP address
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>

/**
 * \brief This function uses C++'s filesystem library to check if a file exists
 * \ingroup error_logger
 */
bool file_exists(const std::string &filename)
{
    return std::experimental::filesystem::exists(filename.c_str());
}

/**
 * \brief Main function of the error logger that checks for missing autostart-related files on the NUC
 * 
 * This file includes a mechanism for a NUC to tell the LCC that an error occured (not all files are there that should have been published by the master PC)
 * -> This software is compiled "self-contained", it includes the cpm library (static linking) rather than linking to it dynamically
 * -> This software already should be installed on the NUCs, in the same folder as lab_autostart.bash 
 * \ingroup error_logger
 */
int main (int argc, char *argv[]) { 
    //Initialize the cpm logger, set domain id etc
    cpm::init(argc, argv);
    cpm::Logging::Instance().set_id("hlc_error_logger"); 

    //Write a message every 10 seconds telling that this NUC does not have all required packages (if that's the case)
    uint64_t callback_period = 10000000000ull;

    //Initialize the timer
    std::shared_ptr<cpm::Timer> timer = std::make_shared<cpm::TimerFD>("hlc_timer", callback_period, 0, false);

    //Same as for the autostart - program that tells the LCC that the NUC is online
    //Get all IP addresses; repeat until there is a valid address, log error whenever none is found
    //Example: ID 13 -> 192.168.1.213
    std::string mask = "192.168.1.2";
    std::string hlc_id = "";
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *address;

    while (hlc_id == "")
    {
        getifaddrs(&ifap);
        for (ifa = ifap; ifa; ifa = ifa->ifa_next)
        {
            if(ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET)
            {
                sa = (struct sockaddr_in *) ifa->ifa_addr;
                address = inet_ntoa(sa->sin_addr);

                //Get address string
                if(address != NULL)
                {
                    std::string addr(address);
                    auto pos = addr.find(mask);
                    if (pos != std::string::npos) {
                        hlc_id = addr.substr(pos + mask.size(), hlc_id.size() - pos - mask.size());
                        break;
                    }
                }
            }
        }
        freeifaddrs(ifap);

        //Log error if the own ID could not yet be determined
        if (hlc_id == "")
        {
            cpm::Logging::Instance().write(
                1,
                "%s", 
                "ID of a NUC could not yet be determined"
            );
        }
    }

    std::cout << "Set ID to " << hlc_id << std::endl;

    //Suppress warning for unused parameter in timer (because we only want to show relevant warnings)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"

    timer->start([&](uint64_t t_now) {
        // Matlab RTI DDS
        if (!std::experimental::filesystem::is_directory("/home/guest/dev/software/cpm_lib/dds_idl_matlab"))
        {
            cpm::Logging::Instance().write(
                1,
                "%s", 
                "The cpm IDL files for matlab are missing on NUC %s", hlc_id.c_str()
            );
        }

        // Matlab eProsima
        if (!std::experimental::filesystem::is_directory("/home/guest/dev/software/cpm_lib/matlab_mex_bindings"))
        {
            cpm::Logging::Instance().write(
                1,
                "%s", 
                "The MEX binding folder for matlab is missing on NUC %s", hlc_id.c_str()
            );
        }

        // Matlab eProsima
        if (!file_exists("/home/guest/dev/software/cpm_lib/matlab_mex_bindings/ready_status_writer.mexa64") ||
            !file_exists("/home/guest/dev/software/cpm_lib/matlab_mex_bindings/system_trigger_reader.mexa64") ||
            !file_exists("/home/guest/dev/software/cpm_lib/matlab_mex_bindings/vehicle_command_trajectory_writer.mexa64") ||
            !file_exists("/home/guest/dev/software/cpm_lib/matlab_mex_bindings/vehicle_state_list_reader.mexa64") ||
            !file_exists("/home/guest/dev/software/cpm_lib/matlab_mex_bindings/vehicleStateListReader.m") ||
            !file_exists("/home/guest/dev/software/cpm_lib/matlab_mex_bindings/systemTriggerReader.m") ||
            !file_exists("/home/guest/dev/software/cpm_lib/matlab_mex_bindings/ReadyStatus.m") ||
            !file_exists("/home/guest/dev/software/cpm_lib/matlab_mex_bindings/TrajectoryPoint.m") ||
            !file_exists("/home/guest/dev/software/cpm_lib/matlab_mex_bindings/VehicleCommandTrajectory.m")
        )
        {
            cpm::Logging::Instance().write(
                1,
                "%s", 
                "The Matlab MEX eProsima files are missing on NUC %s", hlc_id.c_str()
            );
        }

        if (!file_exists("/home/guest/dev/software/cpm_lib/build/libcpm.so"))
        {
            cpm::Logging::Instance().write(
                1,
                "%s", 
                "The cpm library is missing on NUC %s", hlc_id.c_str()
            );
        }

        if (!file_exists("/home/guest/dev/software/middleware/build/middleware"))
        {
            cpm::Logging::Instance().write(
                1,
                "%s", 
                "The middleware executable is missing on NUC %s", hlc_id.c_str()
            );
        }

        if (!file_exists("/home/guest/dev/software/middleware/build/QOS_LOCAL_COMMUNICATION.xml"))
        {
            cpm::Logging::Instance().write(
                1,
                "%s", 
                "The middleware QoS file is missing on NUC %s", hlc_id.c_str()
            );
        }

        if (!file_exists("/home/guest/dev/software/high_level_controller/init_script.m"))
        {
            cpm::Logging::Instance().write(
                1,
                "%s", 
                "The matlab import file is missing on NUC %s", hlc_id.c_str()
            );
        }

        if (!file_exists("/home/guest/dev/software/high_level_controller/read_system_trigger.m"))
        {
            cpm::Logging::Instance().write(
                1,
                "%s", 
                "The matlab read_system_trigger file is missing on NUC %s", hlc_id.c_str()
            );
        }

        if (!file_exists("/home/guest/dev/software/high_level_controller/QOS_READY_TRIGGER.xml"))
        {
            cpm::Logging::Instance().write(
                1,
                "%s", 
                "The matlab QoS file is missing on NUC %s", hlc_id.c_str()
            );
        }

        //This software only is started in case that packages are missing - if none of the conditions above hold, still log a general error message
        cpm::Logging::Instance().write(
            1,
            "%s", 
            "Files are missing on NUC %s", hlc_id.c_str()   
        );
    });

    #pragma GCC diagnostic pop

    return 0;
}
