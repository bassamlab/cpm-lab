#include <iostream>
#include <algorithm>
#include <thread>
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#include "cpm/AsyncReader.hpp"
#include "cpm/ParticipantSingleton.hpp"
#include "cpm/Timer.hpp"

#include "cpm/Writer.hpp"
#include "cpm/Logging.hpp"
#include "cpm/CommandLineReader.hpp"
#include "cpm/init.hpp"
#include "cpm/type_creation_helper.hpp"

#include "cpm/dds/VehicleStatePubSubTypes.h"
#include "cpm/dds/VehicleCommandTrajectoryPubSubTypes.h"

/**
 * \file main_platoon.cpp
 * \brief TODO. Contains the other main function (without subgroups, Doxygen only includes one)
 * \ingroup controller_test_loop
 */

/**
 * \brief The other main function of the controller_test_loop scenario
 * \param argc Command line param
 * \param argv Command line param
 * \ingroup controller_test_loop
 */
int main(int argc, char *argv[])
{
    cpm::init(argc, argv);
    cpm::Logging::Instance().set_id("platoon_controller");
    const bool enable_simulated_time = cpm::cmd_parameter_bool("simulated_time", false, argc, argv);


    std::mutex _mutex;
    std::map<uint8_t, VehicleState> vehicleStates;

    cpm::AsyncReader<VehicleStatePubSubType> vehicleStates_reader(
        [&](std::vector<VehicleState>& samples) {
            std::unique_lock<std::mutex> lock(_mutex);
            for(auto& data : samples) 
            {
                vehicleStates[data.vehicle_id()] = data;
            }
        }, 
        "vehicleState"
    );


    cpm::Writer<VehicleCommandTrajectoryPubSubType> writer_vehicleCommandTrajectory("vehicleCommandTrajectory");


    std::map<uint8_t, uint8_t> follower_vehicle_id_map;

    follower_vehicle_id_map[1] = 2;
    follower_vehicle_id_map[2] = 4;



    // Control loop, which sends trajectory commands to the vehicles
    auto timer = cpm::Timer::create("platoon_controller", 400000000ull, 0, false, true, enable_simulated_time);
    timer->start([&](uint64_t t_now) {
        std::unique_lock<std::mutex> lock(_mutex);

        //const uint64_t t_eval = t_now + 500000000ull;

        for(auto e:vehicleStates)
        {
            uint8_t vehicle_id = e.first;
            auto state = e.second;
            uint64_t t_state = state.header().create_stamp().nanoseconds();


            if(t_state + 3000000000ull < t_now) continue; // ignore old states
            if(follower_vehicle_id_map.count(vehicle_id) == 0) continue; // this vehicle has no follower



            const double c = cos(state.pose().yaw());
            const double s = sin(state.pose().yaw());

            TimeStamp stamp;
            stamp.nanoseconds(t_state + 1200000000ull);

            TrajectoryPoint trajectory_point = cpm::create_trajectory_point(
                stamp, 
                state.pose().x(), 
                state.pose().y(),
                c * state.speed(), 
                s * state.speed()
            );
            VehicleCommandTrajectory vehicleCommandTrajectory;
            vehicleCommandTrajectory.vehicle_id(follower_vehicle_id_map[vehicle_id]);
            vehicleCommandTrajectory.trajectory_points(std::vector<TrajectoryPoint>(1, trajectory_point));
            writer_vehicleCommandTrajectory.write(vehicleCommandTrajectory);

        }


        /*for (size_t slot_idx = 0; slot_idx < slot_vehicle_ids.size(); ++slot_idx)
        {
            const uint8_t vehicle_id = slot_vehicle_ids.at(slot_idx);

            uint64_t trajectory_index = 
                (t_eval + slot_idx * vehicle_time_gap_nanoseconds) / point_period_nanoseconds;

            trajectory_index = trajectory_index % trajectory_points.size();

            auto trajectory_point = trajectory_points.at(trajectory_index);
            trajectory_point.t().nanoseconds(t_eval);


            // slot is assigned, just send the trajectory
            if(vehicle_id > 0)
            {
                // Send trajectory point on DDS
                VehicleCommandTrajectory vehicleCommandTrajectory;
                vehicleCommandTrajectory.vehicle_id(vehicle_id);
                vehicleCommandTrajectory.trajectory_points(rti::core::vector<TrajectoryPoint>(1, trajectory_point));
                writer_vehicleCommandTrajectory.write(vehicleCommandTrajectory);
            }
        }
        unassigned_vehicle_ids.clear();*/
    });

    return 0;
}