

//Mex includes
#include "mex.hpp"
#include "mexAdapter.hpp"

//The Matlab docs specify how to integrate external libraries (.so, with -l in mex call)

//Data types used here
#include "cpm/dds/SystemTriggerPubSubTypes.h"

//Try to use the cpm version of the reader
#include "cpm/init.hpp"
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

        //Init. cpm lib
        char program_name[] = "mex_trigger_reader";
        char arg1[] = "--dds_domain=1";
        char arg2[] = "--logging_id=mex_trigger_reader";
        char *argv[] = {program_name, arg1, arg2};
        int argc = 3;
        cpm::init(argc, argv);

        //Create the participant and DataReader
        participant = std::make_shared<cpm::Participant>(1, true);
        reader = std::make_shared<cpm::ReaderAbstract<SystemTriggerPubSubType>>("systemTrigger", true);

        // //Set participant QoS (shared memory / local communication only)
        // eprosima::fastdds::dds::DomainParticipantQos domain_qos;
        // auto shm_transport = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
        // domain_qos.transport().use_builtin_transports = false;
        // domain_qos.transport().user_transports.push_back(shm_transport);

        // //Create eProsima writer
        // participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        //     1, 
        //     domain_qos
        // );

        // if (participant_ == nullptr)
        // {
        //     matlabPtr->feval(u"error", 0, 
        //         std::vector<matlab::data::Array>({ factory.createScalar("Participant creation failed") }));
        //     return;
        // }

        // // Register the Type
        // type_.register_type(participant_);

        // // Create the publications Topic
        // topic_ = participant_->create_topic("systemTrigger", type_->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

        // if (topic_ == nullptr)
        // {
        //     matlabPtr->feval(u"error", 0, 
        //         std::vector<matlab::data::Array>({ factory.createScalar("Topic creation failed") }));
        //     return;
        // }

        // // Create the Publisher
        // subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);

        // if (subscriber_ == nullptr)
        // {
        //     matlabPtr->feval(u"error", 0, 
        //         std::vector<matlab::data::Array>({ factory.createScalar("Subscriber creation failed") }));
        //     return;
        // }

        // // Set Writer QoS
        // auto qos = eprosima::fastdds::dds::DataReaderQos();
        // auto policy_rel = eprosima::fastdds::dds::ReliabilityQosPolicy();
        // policy_rel.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        // qos.reliability(policy_rel);

        // auto policy_his = eprosima::fastdds::dds::HistoryQosPolicy();
        // policy_his.kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
        // qos.history(policy_his);

        // auto policy_dur = eprosima::fastdds::dds::DurabilityQosPolicy();
        // policy_dur.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        // qos.durability(policy_dur);

        // // Create the DataWriter
        // reader_ = subscriber_->create_datareader(topic_, qos, nullptr);

        // if (reader_ == nullptr)
        // {
        //     matlabPtr->feval(u"error", 0, 
        //         std::vector<matlab::data::Array>({ factory.createScalar("Reader creation failed") }));
        //     return;
        // }

        //Wait a bit to allow for matching
        usleep(500000);
    }

    /**
     * \brief Actual function called by Matlab.
     * Params: SystemTrigger (used to create the output), optional boolean (false: default, don't wait infinitely for messages. true: wait infinitely)
     * Returns: Nothing if nothing was received, else the received SystemTrigger
     * \param outputs Outputs sent to the calling Matlab script after execution. Here: Nothing or a SystemTrigger.
     * \param inputs Inputs given by the calling Matlab script. Here: SystemTrigger and optional bool.
     */
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {   
        //Required for data creation
        matlab::data::ArrayFactory factory;

        //Check if writer creation succeeded
        if (! reader)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Can't receive system trigger because reader creation failed!") }));
        }

        //Check if inputs match the expected input
        checkArguments(outputs, inputs);

        //Check if the reader should wait infinitely
        bool wait = false;
        if (inputs.size() > 1) {
            matlab::data::TypedArray<bool> wait_inf = std::move(inputs[1]);
            wait = wait_inf[0];
        }


        //Get ready status msg
        auto msgs = reader->take();
        while (wait && msgs.size() < 1)
        {
            reader->wait_for_unread_message(std::numeric_limits<unsigned int>::max());
            msgs = reader->take();
        }

        //Return the last read system trigger, set it to not valid if it does not exist
        //Write it to a Matlab object, return it
        matlab::data::Array object = std::move(inputs[0]);

        if (msgs.size() > 0)
        {
            //Get next_start_stamp
            matlabPtr->setProperty(object, u"next_start", factory.createScalar<uint64_t>(msgs.end()->next_start().nanoseconds()));
            matlabPtr->setProperty(object, u"is_valid", factory.createScalar<bool>(true));
        }
        else {
            matlabPtr->setProperty(object, u"next_start", factory.createScalar<uint64_t>(0));
            matlabPtr->setProperty(object, u"is_valid", factory.createScalar<bool>(false));
        }

        //Return
        outputs[0] = object;

        return;
    }

    /**
     * \brief Check if the input matches the expected input
     * \param outputs Outputs sent to the calling Matlab script after execution. Here: False if writing failed, else true.
     * \param inputs Inputs given by the calling Matlab script
     */
    void checkArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        matlab::data::ArrayFactory factory;

        if (inputs.size() > 2 || inputs.size() < 1)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ 
                    factory.createScalar("Wrong input size. Input must be an object of type SystemTrigger. Optionally, a bool may follow to specify if the reader should wait infinitely for a system trigger.") 
            }));
        }

        //Test for correct class
        std::vector<matlab::data::Array> args{ inputs[0], factory.createCharArray("SystemTrigger") };
        matlab::data::TypedArray<bool> result = matlabPtr->feval(u"isa", args);
        if (result[0] != true) {
            matlabPtr->feval(u"error", 0, 
               std::vector<matlab::data::Array>({ factory.createScalar("Input must be an object of type SystemTrigger!") }));
        } 

        //Test for optional bool parameter
        if (inputs.size() > 1)
        {
            if (inputs[1].getType() != matlab::data::ArrayType::LOGICAL)
            {
                matlabPtr->feval(u"error", 0, 
                    std::vector<matlab::data::Array>({ 
                        factory.createScalar("The optional second input must be of type bool. It specifies if the reader should wait for a system trigger infinitely.") 
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