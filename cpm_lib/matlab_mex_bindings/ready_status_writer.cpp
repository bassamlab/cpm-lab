

//Mex includes
#include "mex.hpp"
#include "mexAdapter.hpp"

//The Matlab docs specify how to integrate external libraries (.so, with -l in mex call)

//Data types used here
#include "cpm/dds/ReadyStatusPubSubTypes.h"

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

    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;
    eprosima::fastdds::dds::Topic* topic_ = nullptr;
    eprosima::fastdds::dds::DataWriter* writer_ = nullptr;
    eprosima::fastdds::dds::TypeSupport type_{new ReadyStatusPubSubType()};
public:
    /**
     * \brief Constructor, sets up eprosima objects and matlabPtr
     */
    MexFunction()
    {
        matlabPtr = getEngine();

        matlab::data::ArrayFactory factory;

        //Set participant QoS (shared memory / local communication only)
        eprosima::fastdds::dds::DomainParticipantQos domain_qos;
        auto shm_transport = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
        domain_qos.transport().use_builtin_transports = false;
        domain_qos.transport().user_transports.push_back(shm_transport);

        //Create eProsima writer
        participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
            1, 
            domain_qos
        );

        if (participant_ == nullptr)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Participant creation failed") }));
            return;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the publications Topic
        topic_ = participant_->create_topic("readyStatus", type_->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Topic creation failed") }));
            return;
        }

        // Create the Publisher
        publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);

        if (publisher_ == nullptr)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Publisher creation failed") }));
            return;
        }

        // Set Writer QoS
        auto qos = eprosima::fastdds::dds::DataWriterQos();
        auto policy_rel = eprosima::fastdds::dds::ReliabilityQosPolicy();
        policy_rel.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        qos.reliability(policy_rel);

        auto policy_his = eprosima::fastdds::dds::HistoryQosPolicy();
        policy_his.kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
        qos.history(policy_his);

        auto policy_dur = eprosima::fastdds::dds::DurabilityQosPolicy();
        policy_dur.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        qos.durability(policy_dur);

        // Create the DataWriter
        writer_ = publisher_->create_datawriter(topic_, qos, nullptr);

        if (writer_ == nullptr)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Writer creation failed") }));
            return;
        }

        //Wait a bit to allow for matching
        usleep(500000);
    }

    /**
     * \brief Destructor, destroys the eprosima objects again
     */
    ~MexFunction()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }
        if (publisher_ != nullptr)
        {
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    /**
     * \brief Actual function that gets called when ready_signal_writer gets called within Matlab
     * \param outputs Outputs sent to the calling Matlab script after execution. (Currently empty)
     * \param inputs Inputs given by the calling Matlab script
     */
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {   
        //Required for data creation
        matlab::data::ArrayFactory factory;

        //Check if writer creation succeeded
        if (writer_ == nullptr)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Can't send ready signal because writer creation failed!") }));
        }

        //Check if inputs match the expected input
        checkArguments(outputs, inputs);

        //Write ready status msg
        auto status = get_ready_status_from_input(inputs);
        writer_->write(&status);

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
                std::vector<matlab::data::Array>({ factory.createScalar("Wrong input size. Input must be an object of type ReadyStatus!") }));
        }

        //Test for correct class
        std::vector<matlab::data::Array> args{ inputs[0], factory.createCharArray("ReadyStatus") };
        matlab::data::TypedArray<bool> result = matlabPtr->feval(u"isa", args);
        if (result[0] != true) {
            matlabPtr->feval(u"error", 0, 
               std::vector<matlab::data::Array>({ factory.createScalar("Input must be an object of type ReadyStatus!") }));
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