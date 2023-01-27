#include "VehicleAutomatedControl.hpp"

/**
 * \file VehicleAutomatedControl.cpp
 * \ingroup lcc
 */

/**
 * \brief Maximum number of vehicles.
 * \ingroup lcc
 */
const size_t MAX_NUM_VEHICLES = 30;

VehicleAutomatedControl::VehicleAutomatedControl() 
{

    writer_vehicleCommandSpeedCurvature = make_shared<cpm::Writer<VehicleCommandSpeedCurvaturePubSubType>>("vehicleCommandSpeedCurvature");
    
    std::string command_speed_curvature_topic = "";
    for (size_t vehicle_id = 1; vehicle_id < MAX_NUM_VEHICLES; vehicle_id++)
    {
        command_speed_curvature_topic = "vehicle/" + std::to_string(vehicle_id) + "/vehicleCommandSpeedCurvature";
        writers_vehicleCommandSpeedCurvature.push_back(
            std::unique_ptr<cpm::Writer<VehicleCommandSpeedCurvaturePubSubType>>(
                new cpm::Writer<VehicleCommandSpeedCurvaturePubSubType>(command_speed_curvature_topic)
            )
        );
    }

    //Initialize the timer (task loop) - here, different tasks like stopping the vehicle are performed
    task_loop = std::make_shared<cpm::TimerFD>("LCCAutomatedControl", 200000000ull, 0, false);
    
    //Suppress warning for unused parameter in timer (because we only want to show relevant warnings)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"

    task_loop->start_async([&](uint64_t t_now)
    {
        std::lock_guard<std::mutex> lock(stop_list_mutex);
        for (auto iter = vehicle_stop_list.begin(); iter != vehicle_stop_list.end();)
        {
            //Create and send stop signal for the vehicle - TODO, here test value with speed > 0
            VehicleCommandSpeedCurvature stop_command;
            stop_command.vehicle_id(iter->first);
            stop_command.speed(0);
            stop_command.curvature(0);

            cpm::stamp_message(stop_command, cpm::get_time_ns(), 100000000ull);

            writer_vehicleCommandSpeedCurvature->write(stop_command);

            //Delete the vehicle from the map if its message count is zero - then, enough messages should have been sent
            if (iter->second == 0)
            {
                iter = vehicle_stop_list.erase(iter);
            }
            else 
            {
                iter->second = iter->second - 1;
                ++iter;
            }
        }
    },
    [](){
        //Empty lambda callback for stop signals -> Do nothing when a stop signal is received
    });

    #pragma GCC diagnostic pop
}

void VehicleAutomatedControl::stop_vehicles(std::vector<uint8_t> id_list)
{
    //Create and send stop signal for each vehicle
    for (const auto& id : id_list)
    {
        stop_vehicle(id);
        std::cout << "Stopping " << static_cast<int>(id) << std::endl;
    }
}

void VehicleAutomatedControl::stop_vehicle(uint8_t id)
{
    std::lock_guard<std::mutex> lock(stop_list_mutex);
    vehicle_stop_list[id] = 5; //How often the command should be sent
}