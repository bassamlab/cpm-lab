

//Mex includes
#include "mex.hpp"
#include "mexAdapter.hpp"

//The Matlab docs specify how to integrate external libraries (.so, with -l in mex call)

//Data types used here
#include "cpm/dds/VehicleCommandSpeedCurvaturePubSubTypes.h"

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

    std::shared_ptr<cpm::Participant> participant;
    std::shared_ptr<cpm::Writer<VehicleCommandSpeedCurvaturePubSubType>> writer;
public:
    /**
     * \brief Constructor, sets up matlabPtr, cannot setup eProsima bc we do not know the Domain ID yet
     */
    MexFunction()
    {
        matlabPtr = getEngine();
    }

    /**
     * \brief Actual function that gets called when vehicle_command_... gets called within Matlab
     * \param outputs Outputs sent to the calling Matlab script after execution. (Currently empty)
     * \param inputs Inputs given by the calling Matlab script. Here: The VehicleCommandSpeedCurvature Matlab Object to send. Optional: Matlab Domain ID.
     */
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {   
        //Required for data creation
        matlab::data::ArrayFactory factory;

        //Check if inputs match the expected input
        checkArguments(outputs, inputs);

        //Get domain_id from input, if set
        auto domain_id = 1; //DEFAULT
        if(inputs.size() > 1)
        {
            matlab::data::TypedArray<uint32_t> domain_ids = std::move(inputs[1]);
            domain_id = domain_ids[0];
        }

        //Set up eProsima objects if they do not yet exist
        if (!participant || !writer)
        {
            participant = std::make_shared<cpm::Participant>(
                domain_id, 
                true
            );
            writer = std::make_shared<cpm::Writer<VehicleCommandSpeedCurvaturePubSubType>>(
                participant->get_participant(), 
                "vehicleCommandSpeedCurvature", 
                false, 
                false, 
                false
            );
        }

        //Write ready status msg
        auto status = get_vehicle_command_from_input(inputs);
        writer->write(status);

        return;
    }

    /**
     * \brief Check if the input matches the expected input
     * \param outputs Outputs sent to the calling Matlab script after execution. Here: False if writing failed, else true.
     * \param inputs Inputs given by the calling Matlab script
     */
    void checkArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        matlab::data::ArrayFactory factory;

        if (inputs.size() < 1 || inputs.size() > 2)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Wrong input size. Input must be an object of type VehicleCommandSpeedCurvature, followed by an optional domain_id argument (default is 1)!") }));
        }

        //Test for correct class w.r.t. message to send
        std::vector<matlab::data::Array> args{ inputs[0], factory.createCharArray("VehicleCommandSpeedCurvature") };
        matlab::data::TypedArray<bool> result = matlabPtr->feval(u"isa", args);
        if (result[0] != true) {
            matlabPtr->feval(u"error", 0, 
               std::vector<matlab::data::Array>({ factory.createScalar("Input must be an object of type VehicleCommandSpeedCurvature!") }));
        } 

        //Test for correct class w.r.t. domain_id
        if (inputs.size() > 1)
        {
            if (inputs[1].getType() != matlab::data::ArrayType::UINT32)
            {
                matlabPtr->feval(u"error", 0, 
                    std::vector<matlab::data::Array>({ 
                        factory.createScalar("The optional input domain_id must be of type uint32. It specifies the domain ID of the participant (default is 1).") 
                }));
            }
        }
    }

    /**
     * \brief Reads the ID to send within the ready signal from the input list
     * \param inputs Inputs from the Matlab script that called this object
     */
    VehicleCommandSpeedCurvature get_vehicle_command_from_input(matlab::mex::ArgumentList inputs)
    {
        matlab::data::ArrayFactory factory;

        VehicleCommandSpeedCurvature command;

        //Get "object" from input so that we can access it in C++
        matlab::data::Array object = std::move(inputs[0]);

        //Get vehicle_id
        matlab::data::TypedArray<uint8_t> vehicle_id_ml = matlabPtr->getProperty(object, u"vehicle_id");
        command.vehicle_id(vehicle_id_ml[0]);

        //Get header data
        matlab::data::TypedArray<uint64_t> create_stamp_ml = matlabPtr->getProperty(object, u"create_stamp");
        TimeStamp create_stamp;
        create_stamp.nanoseconds(create_stamp_ml[0]);

        matlab::data::TypedArray<uint64_t> valid_after_stamp_ml = matlabPtr->getProperty(object, u"valid_after_stamp");
        TimeStamp valid_after_stamp;
        valid_after_stamp.nanoseconds(valid_after_stamp_ml[0]);

        Header header;
        header.create_stamp(create_stamp);
        header.valid_after_stamp(valid_after_stamp);

        command.header(header);

        //Get speed curvature data
        matlab::data::TypedArray<double> speed_ml = matlabPtr->getProperty(object, u"speed");
        matlab::data::TypedArray<double> curvature_ml = matlabPtr->getProperty(object, u"curvature");
        command.speed(speed_ml[0]);
        command.curvature(curvature_ml[0]);

        return command;
    }
};