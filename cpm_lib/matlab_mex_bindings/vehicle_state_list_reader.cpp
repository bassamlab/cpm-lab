

//Mex includes
#include "mex.hpp"
#include "mexAdapter.hpp"

//The Matlab docs specify how to integrate external libraries (.so, with -l in mex call)

//Data types used here
#include "cpm/dds/VehicleStateListPubSubTypes.h"

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

    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;
    eprosima::fastdds::dds::Topic* topic_ = nullptr;
    eprosima::fastdds::dds::DataReader* reader_ = nullptr;
    eprosima::fastdds::dds::TypeSupport type_{new VehicleStateListPubSubType()};
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
        topic_ = participant_->create_topic("vehicleStateList", type_->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Topic creation failed") }));
            return;
        }

        // Create the Publisher
        subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);

        if (subscriber_ == nullptr)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Subscriber creation failed") }));
            return;
        }

        // Set Writer QoS
        auto qos = eprosima::fastdds::dds::DataReaderQos();
        auto policy_rel = eprosima::fastdds::dds::ReliabilityQosPolicy();
        policy_rel.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        qos.reliability(policy_rel);

        auto policy_his = eprosima::fastdds::dds::HistoryQosPolicy();
        policy_his.kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS; //We only care about the newest msg
        qos.history(policy_his);

        auto policy_dur = eprosima::fastdds::dds::DurabilityQosPolicy();
        policy_dur.kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        qos.durability(policy_dur);

        // Create the DataWriter
        reader_ = subscriber_->create_datareader(topic_, qos, nullptr);

        if (reader_ == nullptr)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Reader creation failed") }));
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
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    /**
     * \brief Actual function that gets called when ready_signal_writer gets called within Matlab.
     * Params: Just a VehicleStateList object.
     * Ouput: The same object w. is_valid = false if nothing was received, else with the received data.
     * You have to wait for messages in Matlab. If desired, this could also be implemented here, similar to system_trigger_reader.
     * \param outputs Outputs sent to the calling Matlab script after execution. Here: The last received VehicleStateList, with is_valid = false if no msg was received.
     * \param inputs Inputs given by the calling Matlab script. Here: The VehicleStateList Matlab Object to receive.
     */
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {   
        //Required for data creation
        matlab::data::ArrayFactory factory;

        //Check if writer creation succeeded
        if (reader_ == nullptr)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Can't receive state list as the reader creation failed!") }));
        }

        //Check if inputs match the expected input
        checkArguments(outputs, inputs);

        //Create output object from input 
        matlab::data::Array state_list_object = std::move(inputs[0]);
        //Default: No data was received, output is invalid
        matlabPtr->setProperty(state_list_object, u"is_valid", factory.createScalar<bool>(false));

        //Debugging: Print msg type
        // matlabPtr->feval(u"disp", 0, 
        //             std::vector<matlab::data::Array>({ factory.createScalar(type_->getName()) }));

        //Read received msgs, wait up to 5 seconds if none were received yet
        eprosima::fastdds::dds::SampleInfo info;
        VehicleStateList vehicle_state_list;
        std::vector<VehicleStateList> msgs;
        
        while(msgs.size() < 1)
        {
            usleep(10000);
            
            auto retcode = reader_->take_next_sample(&vehicle_state_list, &info);
            print_retcode(retcode); //For debugging

            if (retcode == ReturnCode_t::RETCODE_OK)
            {
                matlabPtr->feval(u"disp", 0, 
                        std::vector<matlab::data::Array>({ factory.createScalar("Received something") }));
                if (info.valid_data)
                {
                    msgs.push_back(vehicle_state_list);

                    matlabPtr->feval(u"disp", 0, 
                        std::vector<matlab::data::Array>({ factory.createScalar("Working on it") }));
                    //Now, vehicle_state_list contains all relevant data
                    //First, start with the data that is easy to copy

                    //We received something, so set is_valid to true
                    matlabPtr->setProperty(state_list_object, u"is_valid", factory.createScalar<bool>(true));

                    //Set everything that is not a list
                    matlabPtr->setProperty(state_list_object, u"t_now", factory.createScalar<uint64_t>(vehicle_state_list.t_now()));
                    matlabPtr->setProperty(state_list_object, u"period_ms", factory.createScalar<uint64_t>(vehicle_state_list.period_ms()));

                    //Set lists
                    write_active_vehicle_ids(state_list_object, vehicle_state_list);
                    write_vehicle_state_list(state_list_object, vehicle_state_list);
                    write_vehicle_observation_list(state_list_object, vehicle_state_list);
                }
            }
        }

        //Return object as output
        outputs[0] = state_list_object;
        return;
    }

    /**
     * \brief For debugging: Print the eprosima return code for received messages
     */
    void print_retcode(ReturnCode_t retcode) {
        matlab::data::ArrayFactory factory;

        if (retcode == ReturnCode_t::RETCODE_OK) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE OK") }));
        }
        if (retcode == ReturnCode_t::RETCODE_ERROR) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE ERROR") }));
        }
        if (retcode == ReturnCode_t::RETCODE_UNSUPPORTED) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE UNSUPPORTED") }));
        }
        if (retcode == ReturnCode_t::RETCODE_BAD_PARAMETER) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE BAD PARAMETER") }));
        }
        if (retcode == ReturnCode_t::RETCODE_PRECONDITION_NOT_MET) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE PRECONDITION NOT MET") }));
        }
        if (retcode == ReturnCode_t::RETCODE_OUT_OF_RESOURCES) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE OUT OF RESOURCES") }));
        }
        if (retcode == ReturnCode_t::RETCODE_NOT_ENABLED) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE NOT ENABLED") }));
        }
        if (retcode == ReturnCode_t::RETCODE_IMMUTABLE_POLICY) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE IMMUTABLE POLICY") }));
        }
        if (retcode == ReturnCode_t::RETCODE_INCONSISTENT_POLICY) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE INCONSISTENT POLICY") }));
        }
        if (retcode == ReturnCode_t::RETCODE_ALREADY_DELETED) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE ALREADY DELETED") }));
        }
        if (retcode == ReturnCode_t::RETCODE_TIMEOUT ) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE TIMEOUT") }));
        }
        if (retcode == ReturnCode_t::RETCODE_NO_DATA ) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE NO DATA") }));
        }
        if (retcode == ReturnCode_t::RETCODE_ILLEGAL_OPERATION ) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE ILLEGAL OPERATION") }));
        }
        if (retcode == ReturnCode_t::RETCODE_NOT_ALLOWED_BY_SECURITY) {
            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("RETCODE NOT ALLOWED BY SECURITY") }));
        }
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
                std::vector<matlab::data::Array>({ factory.createScalar("Wrong input size. Input must be an object of type VehicleStateList!") }));
        }

        //Test for correct class
        std::vector<matlab::data::Array> args{ inputs[0], factory.createCharArray("VehicleStateList") };
        matlab::data::TypedArray<bool> result = matlabPtr->feval(u"isa", args);
        if (result[0] != true) {
            matlabPtr->feval(u"error", 0, 
               std::vector<matlab::data::Array>({ factory.createScalar("Input must be an object of type VehicleStateList!") }));
        } 
    }

    /**
     * \brief Writes the received active_vehicle_ids data to the matlab object
     * \param state_list_object The matlab object
     * \param vehicle_state_list The DDS object
     */
    void write_active_vehicle_ids(matlab::data::Array &state_list_object, VehicleStateList &vehicle_state_list)
    {
        matlab::data::ArrayFactory factory;

        matlab::data::TypedArray<int32_t> active_ids = factory.createArray<int32_t>({1, vehicle_state_list.active_vehicle_ids().size()});
        for (auto i = 0; i < vehicle_state_list.active_vehicle_ids().size(); ++i)
        {
            active_ids[i] = vehicle_state_list.active_vehicle_ids().at(i);
        }

        matlabPtr->setProperty(state_list_object, u"active_vehicle_ids", active_ids);
    }

    /**
     * \brief Writes the received state_list data to the matlab object
     * \param state_list_object The matlab object
     * \param vehicle_state_list The DDS object
     */
    void write_vehicle_state_list(matlab::data::Array &state_list_object, VehicleStateList &vehicle_state_list)
    {
        matlab::data::ArrayFactory factory;
    }

    /**
     * \brief Writes the received vehicle_observation_list data to the matlab object
     * \param state_list_object The matlab object
     * \param vehicle_state_list The DDS object
     */
    void write_vehicle_observation_list(matlab::data::Array &state_list_object, VehicleStateList &vehicle_state_list)
    {
        matlab::data::ArrayFactory factory;
    }
};