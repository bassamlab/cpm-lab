

//Mex includes
#include "mex.hpp"
#include "mexAdapter.hpp"

//The Matlab docs specify how to integrate external libraries (.so, with -l in mex call)

//Data types used here
#include "cpm/dds/VehicleCommandTrajectoryPubSubTypes.h"

#include "cpm/Participant.hpp"
#include "cpm/Writer.hpp"

//Required eProsima libraries
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

//Standard libraries
#include <memory>
#include <string>
#include <unistd.h>

//Kompilieren lässt sich die Datei zurzeit wie folgt: (Pfade bitte entsprechend anpassen)
// /usr/local/MATLAB/R2021a/bin/mex ready_status_writer.cpp -L/home/leon/dev/software/cpm_lib/build/ -lcpm -I/home/leon/dev/software/cpm_lib/include -L/usr/local/lib -lfastcdr -lfastrtps

class MexFunction : public matlab::mex::Function {
private:
    //! Used for translation, printing etc.
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr;

    cpm::Participant participant;
    cpm::Writer<VehicleCommandTrajectoryPubSubType> writer;
public:
    /**
     * \brief Constructor, sets up eprosima objects and matlabPtr
     */
    MexFunction() :
        participant(1, true),
        writer(participant.get_participant(), "vehicleCommandTrajectory", false, false, false)
    {
        matlabPtr = getEngine();

        //Wait a bit to allow for matching
        usleep(500000);
    }

    /**
     * \brief Actual function that gets called when ready_signal_writer gets called within Matlab
     * \param outputs Outputs sent to the calling Matlab script after execution. (Currently empty)
     * \param inputs Inputs given by the calling Matlab script. Here: The ReadyStatus Matlab Object to send.
     */
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {   
        //Required for data creation
        matlab::data::ArrayFactory factory;

        //Check if inputs match the expected input
        checkArguments(outputs, inputs);

        //Write ready status msg
        auto status = get_vehicle_command_from_input(inputs);
        writer.write(status);

        //Return true
        //outputs[0] = factory.createScalar<bool>(true);
        return;
    }

    /**
     * \brief Check if the input matches the expected input
     * \param outputs Outputs sent to the calling Matlab script after execution. Here: False if writing failed, else true.
     * \param inputs Inputs given by the calling Matlab script
     */
    void checkArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        matlab::data::ArrayFactory factory;

        if (inputs.size() != 1)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Wrong input size. Input must be an object of type VehicleCommandTrajectory!") }));
        }

        //Test for correct class
        std::vector<matlab::data::Array> args{ inputs[0], factory.createCharArray("VehicleCommandTrajectory") };
        matlab::data::TypedArray<bool> result = matlabPtr->feval(u"isa", args);
        if (result[0] != true) {
            matlabPtr->feval(u"error", 0, 
               std::vector<matlab::data::Array>({ factory.createScalar("Input must be an object of type VehicleCommandTrajectory!") }));
        } 
    }

    /**
     * \brief Reads the ID to send within the ready signal from the input list
     * \param inputs Inputs from the Matlab script that called this object
     */
    VehicleCommandTrajectory get_vehicle_command_from_input(matlab::mex::ArgumentList inputs)
    {
        VehicleCommandTrajectory command;

        //Get "object" from input so that we can access it in C++
        matlab::data::Array object = std::move(inputs[0]);

        //Get vehicle_id
        matlab::data::TypedArray<uint8_t> vehicle_id = matlabPtr->getProperty(object, u"vehicle_id");
        command.vehicle_id(vehicle_id[0]);

        //Get header data
        matlab::data::TypedArray<uint64_t> create_st = matlabPtr->getProperty(object, u"create_stamp");
        TimeStamp create_stamp;
        create_stamp.nanoseconds(create_st[0]);

        matlab::data::TypedArray<uint64_t> valid_after_st = matlabPtr->getProperty(object, u"valid_after_stamp");
        TimeStamp valid_after_stamp;
        valid_after_stamp.nanoseconds(valid_after_st[0]);

        Header header;
        header.create_stamp(create_stamp);
        header.valid_after_stamp(valid_after_stamp);

        command.header(header);

        //Get trajectory data
        TODO

        return ready_status;
    }
};