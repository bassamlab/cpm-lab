#include <iostream>
#include <vector>
#include <string> 
#include "VehicleTrajectoryPlanner.hpp"
#include "VehicleStatePubSubTypes.h"
#include "Pose2DPubSubTypes.h"
#include "lane_graph_tools.hpp"
#include "cpm/Logging.hpp"                      //->cpm_lib->include->cpm
#include "cpm/CommandLineReader.hpp"            //->cpm_lib->include->cpm
#include "cpm/init.hpp"                         //->cpm_lib->include->cpm
#include "cpm/ParticipantSingleton.hpp"         //->cpm_lib->include->cpm
#include "cpm/Timer.hpp"                        //->cpm_lib->include->cpm
#include "cpm/Writer.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/Reader.hpp"
#include "cpm/HLCCommunicator.hpp"
#include "cpm/Participant.hpp"
#include <atomic>
#include <thread>

void simulate(std::shared_ptr<VehicleTrajectoryPlanner> planner);

// allows for syncing the hlcs that will run multithreaded
std::atomic<int> iterations = 0;

int n_vehicles;
int n_steps;

// vehicles always plan for dt segments (in ns) 
uint64_t dt = 400000000;

std::vector<std::vector<float_t>> start_poses_raw( {{3.15802,3.88862,-0.0507011}, {3.8791,3.72123,-0.497375}, 
                                    {4.35882,2.95562,-1.41334}, {4.39471,1.99965,-1.56801}, 
                                    {4.35932,1.04568,-1.72724}, {3.87964,0.278448,-2.64031}, 
                                    {3.15654,0.111178,-3.09329}, {2.24999,0.104926,3.14091},
                                    {1.34368,0.111556,3.08998}, {0.62071,0.278691,2.63872},
                                    {0.141397,1.04581,1.7287}, {0.104006,2.00092,1.57371},
                                    {0.141131,2.9547,1.4113}, {0.62021,3.72161,0.500138},
                                    {1.34238,3.88924,0.0494666}, {3.05031,2.22542,3.14112},
                                    {3.05088,1.77524,-0.00189507}, {2.47512,1.19944,1.57269},
                                    {2.02503,1.19963,-1.57305}, {1.4504,1.77449,-0.0020518}});

// start poses on the commonroad map (central routing map)
std::vector<Pose2D>  start_poses;
/*
*   Simulates a vehicle for n_steps. Synced to the other vehicle via iterations.
*/
void simulate(std::shared_ptr<VehicleTrajectoryPlanner> planner){
    uint64_t t = 10; // initial time !=0
    while(iterations <= n_vehicles * n_steps)
    {
        if (iterations % n_vehicles == 0)
        {
            planner->plan(t, dt);
            t +=dt;
            std::cout << "TIME: " << t << std::endl;
            iterations++;
        }
    }
    
}

/*
*   Allows for the dynamic_priorities hlc to be simulated at some interval for a given amount of steps (each step covers dt).
*   In modes Static, Random, FCA: (0,1,2)
*   For n vehicles.
*   (logs will be written by the planner instances)
*/
int main(int argc, char* argv[])
{
    Pose2D b_pose; //buffer
    for (auto &r_pose : start_poses_raw)
    {
        b_pose.x(r_pose[0]);
        b_pose.y(r_pose[1]);
        b_pose.yaw(r_pose[2]);
        start_poses.push_back(b_pose);
    }

    std::vector<std::string> arguments = {"dds_domain=21", "dds_initial_peer=rtps@udpv4://192.168.1.249:25598"};

    n_vehicles = cpm::cmd_parameter_int("n", 5, argc, argv);
    const PriorityMode mode = static_cast<PriorityMode>(
        cpm::cmd_parameter_int(
            "hlc_mode", static_cast<int>(PriorityMode::fca), argc, argv
        )
    );
    n_steps = cpm::cmd_parameter_int("steps", 450, argc, argv);

    int prio_seed = cpm::cmd_parameter_int("prio_seed", 0, argc, argv);
    int path_seed = cpm::cmd_parameter_int("path_seed", 0, argc, argv);
    std::cout << "seed base: " << prio_seed << std::endl;
    // create vehicle planners
    std::vector<std::shared_ptr<VehicleTrajectoryPlanner>> vehicles;
    std::vector<uint8_t> vec;
    uint8_t id = 0;
    for (int i = 0; i < n_vehicles; i++)
    {
        id ++;
        vec.push_back(id);
        vehicles.push_back(std::move(std::shared_ptr<VehicleTrajectoryPlanner>(new VehicleTrajectoryPlanner(mode, prio_seed + id))));
        cpm::Logging::Instance().set_id("dynamic_priorities");
    }

    // setup planners
    uint8_t vehicle_id = 0;
    for (auto&& planner : vehicles)
    {
        vehicle_id++;
        // Writer to communicate plans with other vehicles
        cpm::Writer<TrajectoryPubSubType> writer_trajectory(
            "trajectory");
        // Reader to receive planned trajectories of other vehicles
        cpm::ReaderAbstract<TrajectoryPubSubType> reader_trajectory(
            "trajectory");
        
        planner->set_writer(
            std::unique_ptr<cpm::Writer<TrajectoryPubSubType>>(
                new cpm::Writer<TrajectoryPubSubType>( "trajectory")));
        planner->set_reader(
            std::unique_ptr<cpm::ReaderAbstract<TrajectoryPubSubType>>(
                new cpm::ReaderAbstract<TrajectoryPubSubType>( "trajectory")));
        // set fca reader and writer
        planner->set_fca_reader(
            std::unique_ptr<cpm::ReaderAbstract<FutureCollisionAssessmentPubSubType>>(
                new cpm::ReaderAbstract<FutureCollisionAssessmentPubSubType>("futureCollisionAssessment")));
        planner->set_fca_writer(
            std::unique_ptr<cpm::Writer<FutureCollisionAssessmentPubSubType>>(
                new cpm::Writer<FutureCollisionAssessmentPubSubType>("futureCollisionAssessment")));

        // Writer to send visualization to middleware
        planner->set_visualization_writer(
            std::unique_ptr<cpm::Writer<VisualizationPubSubType>>(
                new cpm::Writer<VisualizationPubSubType>(
                    "visualization")));

    
        // sets the vehicles up at the starting poses, mathes them to the nodes of the map graph (include/lane_graph)
        Pose2D pose = start_poses.at(vehicle_id - 1); // ids start at 1; start_poses at 0

        int out_edge_index = -1;
        int out_edge_path_index = -1;
        bool matched = false;
        matched = laneGraphTools.map_match_pose(pose, out_edge_index, out_edge_path_index);
        if (!matched)
        {
        }
        else
        {
            CouplingGraph coupling_graph(vec);

            planner->set_coupling_graph(coupling_graph);
            // Initialize PlanningState with starting position
            planner->set_vehicle(
                std::unique_ptr<VehicleTrajectoryPlanningState>(
                    new VehicleTrajectoryPlanningState(
                        vehicle_id,
                        out_edge_index,
                        out_edge_path_index,
                        dt,
                        path_seed + vehicle_id
                    )
                )
            );
        }
    }

    // creates a thread for each vehicle, it lives for the duration of the simulation
    std::vector<std::thread> threads;
    threads.reserve(n_vehicles);
    for (auto planner : vehicles)
    {
        threads.emplace_back(simulate, planner);
    }
    // wait for the threads to finish
    for (auto &thread : threads)
    {
        thread.join();
    }
}