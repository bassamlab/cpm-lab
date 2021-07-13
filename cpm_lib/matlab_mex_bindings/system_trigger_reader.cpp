

//Mex includes
#include "mex.hpp"
#include "mexAdapter.hpp"

//The Matlab docs specify how to integrate external libraries (.so, with -l in mex call)

//Data types used here
#include "cpm/dds/SystemTriggerPubSubTypes.h"

//Try to use the cpm version of the reader
// #include "cpm/init.hpp"
#include "cpm/Participant.hpp"
#include "cpm/ReaderAbstract.hpp"

//Required eProsima libraries
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
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

    // eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    // eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;
    // eprosima::fastdds::dds::Topic* topic_ = nullptr;
    // eprosima::fastdds::dds::DataReader* reader_ = nullptr;
    // eprosima::fastdds::dds::TypeSupport type_{new SystemTriggerPubSubType()};

    std::shared_ptr<cpm::Participant> participant;
    std::shared_ptr<cpm::ReaderAbstract<SystemTriggerPubSubType>> reader;
public:
    /**
     * \brief Constructor, sets up eprosima objects and matlabPtr
     */
    MexFunction()
    {
        matlabPtr = getEngine();

        matlab::data::ArrayFactory factory;

        // //Init. cpm lib
        // char program_name[] = "mex_trigger_reader";
        // char arg1[] = "--dds_domain=1";
        // char arg2[] = "--logging_id=mex_trigger_reader";
        // char *argv[] = {program_name, arg1, arg2};
        // int argc = 3;
        // cpm::init(argc, argv);
    }

    /**
     * \brief Actual function called by Matlab.
     * Params: Optional boolean (false: default, don't wait infinitely for messages. true: wait infinitely (max. unsigned integer milliseconds))
     * Returns: Nothing if nothing was received, else the received SystemTrigger
     * \param outputs Outputs sent to the calling Matlab script after execution. Here: The last received SystemTrigger, with is_valid = false if no msg was received.
     * \param inputs Inputs given by the calling Matlab script. Here: SystemTrigger and optional bool.
     */
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {   
        //Required for data creation
        matlab::data::ArrayFactory factory;

        //Check if inputs match the expected input
        checkArguments(outputs, inputs);

        //Get domain_id from input, if set
        auto domain_id = 1; //DEFAULT
        if(inputs.size() > 0)
        {
            matlab::data::TypedArray<uint32_t> domain_ids = std::move(inputs[0]);
            domain_id = domain_ids[0];
        }

        //Create the DDS participants, if they do not already exist
        //Needs to be done here to be able to set the domain ID
        if (!participant || !reader)
        {
            participant = std::make_shared<cpm::Participant>(
                domain_id, 
                true
            );
            reader = std::make_shared<cpm::ReaderAbstract<SystemTriggerPubSubType>>(
                participant->get_participant(), 
                "systemTrigger", 
                true
            );
        }

        //Check if the reader should wait infinitely
        bool wait = false;
        if (inputs.size() > 1) {
            matlab::data::TypedArray<bool> wait_inf = std::move(inputs[1]);
            wait = wait_inf[0];
        }

        //Wait inf. if desired
        if(wait) {
            reader->wait_for_unread_message(std::numeric_limits<unsigned int>::max());
        }

        //Get ready status msg
        auto msgs = reader->take();

        //Return the last read system trigger, set it to not valid if it does not exist
        //Create output struct
        matlab::data::StructArray system_trigger_object = factory.createStructArray(
            {1, 1},
            {"next_start", 
            "is_valid"});

        if (msgs.size() > 0)
        {
            //Get next_start_stamp
            system_trigger_object[0]["next_start"] = factory.createScalar<uint64_t>(msgs.rbegin()->next_start().nanoseconds());
            system_trigger_object[0]["is_valid"] = factory.createScalar<bool>(true);
        }
        else {
            system_trigger_object[0]["next_start"] = factory.createScalar<uint64_t>(0);
            system_trigger_object[0]["is_valid"] = factory.createScalar<bool>(false);
        }

        //Return
        outputs[0] = system_trigger_object;

        return;
    }

    /**
     * \brief Check if the input matches the expected input
     * \param outputs Outputs sent to the calling Matlab script after execution. Here: False if writing failed, else true.
     * \param inputs Inputs given by the calling Matlab script
     */
    void checkArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        matlab::data::ArrayFactory factory;

        if (inputs.size() > 2)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ 
                    factory.createScalar("Wrong input size. The first optional parameter is the domain ID (default is 1). Optionally, a following bool may be used to specify if the reader should wait infinitely for a system trigger.") 
            }));
        }

        //Test for optional bool parameter
        if (inputs.size() >= 2)
        {
            if (inputs[1].getType() != matlab::data::ArrayType::LOGICAL)
            {
                matlabPtr->feval(u"error", 0, 
                    std::vector<matlab::data::Array>({ 
                        factory.createScalar("The second optional input must be of type bool. It specifies if the reader should wait for a system trigger infinitely (or rather, for years, which should be sufficient).") 
                }));
            }
        }

        //Test for correct class w.r.t. domain_id
        if (inputs.size() >= 1)
        {
            if (inputs[0].getType() != matlab::data::ArrayType::UINT32)
            {
                matlabPtr->feval(u"error", 0, 
                    std::vector<matlab::data::Array>({ 
                        factory.createScalar("The first optional input domain_id must be of type uint32. It specifies the domain ID of the participant (default is 1).") 
                }));
            }
        }
    }

    /**
     * \brief Reads the ID to send within the ready signal from the input list
     * \param inputs Inputs from the Matlab script that called this object
     */
    // SystemTrigger get_system_trigger_from_input(matlab::mex::ArgumentList inputs)
    // {
    //     SystemTrigger system_trigger;

    //     //Get "object" from input so that we can access it in C++
    //     matlab::data::Array object = std::move(inputs[0]);

    //     //Get next_start_stamp
    //     matlab::data::TypedArray<uint64_t> next_start = matlabPtr->getProperty(object, u"next_start");
    //     TimeStamp stamp;
    //     stamp.nanoseconds(next_start[0]);
    //     system_trigger.next_start(stamp);

    //     return system_trigger;
    // }
};