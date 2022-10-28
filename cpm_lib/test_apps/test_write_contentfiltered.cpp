#include "cpm/Logging.hpp"
#include "cpm/ParticipantSingleton.hpp"
#include "cpm/dds/VehicleStatePubSubTypes.h"
#include "cpm/dds/VehicleObservationPubSubTypes.h"
#include "cpm/dds/VehicleCommandDirectPubSubTypes.h"
#include "cpm/dds/ColorPubSubTypes.h"

#include "cpm/Reader.hpp"
#include "cpm/Writer.hpp"
#include "cpm/InternalConfiguration.hpp"
#include <unistd.h>

int main(int argc, char *argv[]){
    //cpm::init(argc, argv);
    cpm::InternalConfiguration::init(argc, argv);
    cpm::Writer<VehicleStatePubSubType> writer_vehicleState("vehicleState");
    cpm::Writer<VehicleObservationPubSubType> writer_vehicleObservation("vehicleObservation");
    cpm::Writer<ColorPubSubType> writer_color("color");
    cpm::Writer<VehicleCommandDirectPubSubType> writer_vehCommand("");

    for (size_t i = 0; i < 10; i++)
    {
         {
            VehicleState vehicleState;
            vehicleState.odometer_distance(123);
            vehicleState.vehicle_id(3);
            writer_vehicleState.write(vehicleState);
        }

        {
            VehicleObservation vehicleObservation;
            vehicleObservation.vehicle_id(3);
            
            writer_vehicleObservation.write(vehicleObservation);
        }

        {
            Color c;
            c.a(1);
            c.r(2);
            c.g(3);
            c.b(4);
            
            writer_color.write(c);
        }

        {
            VehicleCommandDirect vehicleCommand;
            vehicleCommand.vehicle_id(3);
            writer_vehCommand.write(vehicleCommand);
        }
        

        usleep(500000);
    }
    
    
}