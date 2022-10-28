#include "cpm/Logging.hpp"
#include "cpm/ParticipantSingleton.hpp"
#include "cpm/dds/VehicleStatePubSubTypes.h"
//#include "cpm/dds/VehicleObservationPubSubTypes.h"
#include "cpm/dds/VehicleStateTypeObject.h"

#include "cpm/Reader.hpp"
#include "cpm/ReaderAbstract.hpp"
#include "cpm/MultiVehicleReader.hpp"
#include "cpm/Writer.hpp"
#include "cpm/InternalConfiguration.hpp"
#include <unistd.h>

using VehicleTrajectories = std::map<uint8_t, VehicleState >;

int main(int argc, char *argv[]){
    cpm::InternalConfiguration::init(argc, argv);
    //registerVehicleObservationTypes();
    registerVehicleStateTypes();
    // registerVehicleCommandTrajectoryTypes();
    // registerVehicleCommandPathTrackingTypes();
    //cpm::init(argc, argv);

    cpm::Reader<VehicleStatePubSubType> reader_state("vehicleState", 3);
    // cpm::Reader<VehicleObservationPubSubType> reader_observation("vehicleObservation");
    cpm::ReaderAbstract<VehicleStatePubSubType> reader_all("vehicleState", false, true, false);
    std::shared_ptr<cpm::MultiVehicleReader<VehicleStatePubSubType>> reader_multi;

    std::vector<uint8_t> vehicle_ids = {1,2,3,4};

    reader_multi = std::make_shared<cpm::MultiVehicleReader<VehicleStatePubSubType>>("vehicleState", vehicle_ids);


    for (size_t i = 0; i < 100; i++)
    {
        auto reader_samples_state_dds = reader_state.get_all_samples();

        std::map<uint8_t,  VehicleState> state_samples;
        std::map<uint8_t, uint64_t> state_sample_age;

        reader_multi->get_samples(cpm::get_time_ns(), state_samples, state_sample_age);

        auto reader_samples_all_dds = reader_all.take();

        for (auto& sample : reader_samples_state_dds)
        {
            std::cout << sample.vehicle_id() << ",";
        }
        std::cout << std::endl << "obs: ";
        for (auto& sample : state_samples)
        {
            std::cout << sample.first << ",";
        }   
        std::cout << std::endl << "all: ";
        for (auto& sample : reader_samples_all_dds)
        {
            std::cout << sample.vehicle_id() << ",";
        }
        std::cout << std::endl << "state: ";
        usleep(200000);
    }
    
    
}