

//Mex includes
#include "mex.hpp"
#include "mexAdapter.hpp"

//The Matlab docs specify how to integrate external libraries (.so, with -l in mex call)

//Data types used here
#include "cpm/dds/ReadyStatusPubSubTypes.h"

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
    std::shared_ptr<cpm::Writer<ReadyStatusPubSubType>> writer;
public:
    /**
     * \brief Constructor, sets up eprosima objects and matlabPtr
     */
    MexFunction()
    {
        matlabPtr = getEngine();
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
            writer = std::make_shared<cpm::Writer<ReadyStatusPubSubType>>(
                participant->get_participant(), 
                "readyStatus", 
                true, 
                true, 
                true
            );
        }

        //Write ready status msg
        auto status = get_ready_status_from_input(inputs);
        writer->write(status);

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

        if (inputs.size() < 1 || inputs.size() > 2)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Wrong input size. Input must be an object of type ReadyStatus and an optional domain_id (default is 1)!") }));
        }

        //Test for correct class w.r.t. message to send
        std::vector<matlab::data::Array> args{ inputs[0], factory.createCharArray("ReadyStatus") };
        matlab::data::TypedArray<bool> result = matlabPtr->feval(u"isa", args);
        if (result[0] != true) {
            matlabPtr->feval(u"error", 0, 
               std::vector<matlab::data::Array>({ factory.createScalar("Input must be an object of type ReadyStatus!") }));
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
    ReadyStatus get_ready_status_from_input(matlab::mex::ArgumentList inputs)
    {
        ReadyStatus ready_status;

        //Get "object" from input so that we can access it in C++
        matlab::data::Array object = std::move(inputs[0]);

        //Get source_id
        matlab::data::StringArray source_id = matlabPtr->getProperty(object, u"source_id");
        ready_status.source_id(source_id[0]);

        //Get next_start_stamp
        matlab::data::TypedArray<uint64_t> next_start_stamp = matlabPtr->getProperty(object, u"next_start_stamp");
        TimeStamp stamp;
        stamp.nanoseconds(next_start_stamp[0]);
        ready_status.next_start_stamp(stamp);

        return ready_status;
    }
};