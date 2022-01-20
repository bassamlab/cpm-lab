// Set to true to get additional information about execution time in stdout
#define TIMED true

#include "lane_graph.hpp"                       //sw-folder central routing->include
#include "lane_graph_tools.hpp"                 //sw-folder central routing

// CPM Wrappers and functions
#include "cpm/Logging.hpp"                      //->cpm_lib->include->cpm
#include "cpm/CommandLineReader.hpp"            //->cpm_lib->include->cpm
#include "cpm/init.hpp"                         //->cpm_lib->include->cpm
#include "cpm/ParticipantSingleton.hpp"         //->cpm_lib->include->cpm
#include "cpm/Timer.hpp"                        //->cpm_lib->include->cpm
#include "cpm/get_topic.hpp"
#include "cpm/Writer.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/HLCCommunicator.hpp"

// IDL files
#include "VehicleCommandTrajectory.hpp"
#include "VehicleStateList.hpp"

// General C++ libs
#include <chrono>
#include <iostream>
#include <sstream>
#include <memory>
#include <mutex>
#include <thread>
#include <limits>                           // To get maximum integer value (for stop condition)
#include <exception>

#include "CouplingGraph.hpp"
// Planner
#include "VehicleTrajectoryPlanner.hpp"    //sw-folder central routing

using std::vector;

//Description for bash files
/**
 * \defgroup distributed_routing_files Additional Files
 * \ingroup distributed_routing
 */

/**
 * \page distributed_routing_files_page Additional Files for distributed Routing
 * \subpage d_r_build <br>
 * \subpage d_r_run <br>
 * \subpage d_r_run_distr <br>
 * \ingroup distributed_routing_files
*/

/**
 * \page d_r_build build.bash
 * \brief Build script for distributed_routing
 */

/**
 * \page d_r_run run.bash
 * \brief Default run script for decentral_routing, do not use.
 */

/**
 * \page d_r_run_distr run_distributed.bash
 * \brief Run script to start one or more decentral HLCs with middlewares.
 */

/**
 * \brief Main method to start a decentral HLC
 * \ingroup decentral_routing
 */
int main(int argc, char *argv[]) {   

    //We need to get the path to the executable, and argv[0] is not
    //reliable enough for that (and sometimes also only returns a relative path)
    std::array<char, 128> buffer;
    std::string absolute_executable_path;
    ssize_t len = ::readlink("/proc/self/exe", buffer.data(), buffer.size()-1);
    if (len >= 0) {
      buffer[len] = '\0';
      std::string temp(buffer.data());
      absolute_executable_path = temp;
    }
    else
    {
        std::cerr << "ERROR: Could not obtain executable path, thus deploying functions would not work. Shutting down..." << std::endl;
        exit(EXIT_FAILURE);
    }
    //Get from the executable path to the software folder path of the whole project / repo
    //Executable path: .../software/high_level_controller/examples/cpp/decentral_routing/build/name_of_executable
    //-> Remove everything up to the third-last slash
    auto software_folder_path = absolute_executable_path;
    for (int i = 0; i < 6; ++i)
    {
        auto last_slash = software_folder_path.find_last_of('/');
        if (last_slash != std::string::npos)
        {
            software_folder_path = software_folder_path.substr(0, last_slash);
        }
    }

    // Read command line arguments
    const std::vector<int> vehicle_ids_int = cpm::cmd_parameter_ints("vehicle_ids", {4}, argc, argv);

    cpm::init(argc, argv);

    // Validate given vehicle id(s)
    uint8_t vehicle_id;

    assert(vehicle_ids_int.size()==1);

    std::vector<uint8_t> vehicle_ids;
    for(auto i:vehicle_ids_int)
    {
        assert(i>0);
        assert(i<255);
        vehicle_id = i;
        vehicle_ids.push_back(i);
    }
    cpm::Logging::Instance().set_id("distributed_routing_"+std::to_string(vehicle_id));

    cpm::Logging::Instance().write(3,
            "distributed_routing_%d coming online",
            static_cast<int>(vehicle_id)
    );


    // Outstream in shell which vehicles were selected
    std::stringstream vehicle_id_stream;
    vehicle_id_stream << "Started HLC for Vehicle ID: ";
    vehicle_id_stream << static_cast<uint32_t>(vehicle_id); //Cast s.t. uint8_t is not interpreted as a character
    std::string vehicle_id_string = vehicle_id_stream.str();

    std::cout << vehicle_id_string << std::endl;

    const bool iterative_planning_enabled = true;

    // FIXME: Dirty hack to get our QOS settings
    // On the NUC we only have the QOS File for the middleware
    // and RTI DDS doesn't want to load it from there, so we copy it to our working dir.
    std::stringstream command_stream;
    command_stream << "cp " << software_folder_path << "/middleware/build/QOS_LOCAL_COMMUNICATION.xml .";
    system(command_stream.str().c_str());

    // Initialize everything needed for communication with middleware
    const int middleware_domain = cpm::cmd_parameter_int("middleware_domain", 1, argc, argv);

    /* --------------------------------------------------------------------------------- 
     * Create HLCCommunicator and a writer for communication with middleware
     * ---------------------------------------------------------------------------------
     */
    HLCCommunicator hlc_communicator(
            vehicle_id,
            middleware_domain
    );

    // Writer to send trajectory to middleware
    cpm::Writer<VehicleCommandTrajectory> writer_vehicleCommandTrajectory(
            hlc_communicator.getLocalParticipant()->get_participant(),
            "vehicleCommandTrajectory"
    );

    /* --------------------------------------------------------------------------------- 
     * Reader/Writers for direct comms between vehicles
     * ---------------------------------------------------------------------------------
     */
    // Writer to communicate plans with other vehicles
    cpm::Writer<HlcCommunication> writer_HlcCommunication(
            "hlcCommunication");

    // Reader to receive planned trajectories of other vehicles
    cpm::ReaderAbstract<HlcCommunication> reader_HlcCommunication(
            "hlcCommunication");
    
    /* ---------------------------------------------------------------------------------
     * Create planner object
     * ---------------------------------------------------------------------------------
     */
    auto planner = std::unique_ptr<VehicleTrajectoryPlanner>(new VehicleTrajectoryPlanner());

    // Set reader/writers of planner so it can communicate with other planners
    planner->set_writer(
    std::unique_ptr<cpm::Writer<HlcCommunication>>(
        new cpm::Writer<HlcCommunication>("hlcCommunication")
        )
    );
    planner->set_reader(
    std::unique_ptr<cpm::ReaderAbstract<HlcCommunication>>(
        new cpm::ReaderAbstract<HlcCommunication>("hlcCommunication")
        )
    );

    /* ---------------------------------------------------------------------------------
     * Set the appropriate callback methods on the HLCCommunicator
     * ---------------------------------------------------------------------------------
     */

    /*
     * This is additional initial setup of the planner.
     * We cannot do this before we received a VehicleStateList, but we only want to do it once.
     * We use the 'onFirstTimestep' method of the HLCCommunicator for this.
     * This will get executed just before 'onEachTimestep' is executed for the first time.
     */
    hlc_communicator.onFirstTimestep([&](VehicleStateList vehicle_state_list){
            bool matched = false;
            for(auto vehicle_state : vehicle_state_list.state_list())
            {
                if( vehicle_id == vehicle_state.vehicle_id() ) {
                    auto pose = vehicle_state.pose();
                    int out_edge_index = -1;
                    int out_edge_path_index = -1;
                    matched = laneGraphTools.map_match_pose(pose, out_edge_index, out_edge_path_index);
                    if( !matched ) {
                        cpm::Logging::Instance().write(1,
                            "Couldn't find starting position,\
                            try moving the vehicle.");
                    } else {

                        // This graphs gives the priorities, as well as the order of planning
                        // Currently we only plan sequentially, with lower vehicle ids first
                        std::vector<int> vec(vehicle_state_list.active_vehicle_ids());
                        CouplingGraph coupling_graph(vec);
                        // For testing, make all planning one iterative block
                        if( iterative_planning_enabled ) {
                            coupling_graph.addIterativeBlock(std::vector<int>( vec.begin(), vec.end()));
                        }

                        planner->set_coupling_graph(coupling_graph);

                        // Initialize PlanningState with starting position
                        planner->set_vehicle(
                                std::unique_ptr<VehicleTrajectoryPlanningState>(
                                    new VehicleTrajectoryPlanningState(
                                        vehicle_id,
                                        out_edge_index,
                                        out_edge_path_index
                                        )
                                )
                        );
                        cpm::Logging::Instance().write(
                                2,
                                "Vehicle %d matched.",
                                int(vehicle_id)
                        );
                    }
                }
            }
            });
    hlc_communicator.onEachTimestep([&](VehicleStateList vehicle_state_list){
                try{
                    auto trajectory = planner->plan(vehicle_state_list.t_now(), vehicle_state_list.period_ms()*1e6);
                    if( trajectory.get() != nullptr ) {
                        writer_vehicleCommandTrajectory.write(*trajectory.get());
                    } else {
                        cpm::Logging::Instance().write(2,
                                "Planner didn't return a value this timestep");
                    }
                }
                catch( const std::runtime_error& e ) {
                    cpm::Logging::Instance().write(1,
                            "Planner encountered exception: %s",
                            e.what()
                    );
                    // Tell everyone to stop
                    hlc_communicator.stop(vehicle_id);
                }
            });
    hlc_communicator.onCancelTimestep([&]{
                planner->stop(); 
            });
    hlc_communicator.onStop([&]{
                planner->stop(); 
            });

    hlc_communicator.start();

    return 0;
}
