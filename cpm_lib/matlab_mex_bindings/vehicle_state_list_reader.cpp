

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

#include "cpm/ReaderAbstract.hpp"
#include "cpm/Participant.hpp"

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
    std::shared_ptr<cpm::ReaderAbstract<VehicleStateListPubSubType>> reader;
public:
    /**
     * \brief Constructor, sets up eprosima objects and matlabPtr
     */
    MexFunction()
    {
        matlabPtr = getEngine();
    }

    /**
     * \brief Actual function that gets called when vehicle_state_list_reader gets called within Matlab.
     * Ouput: VehicleStateList w. is_valid = false if nothing was received, else with the received data.
     * You have to wait for messages in Matlab. If desired, this could also be implemented here, similar to system_trigger_reader.
     * \param outputs Outputs sent to the calling Matlab script after execution. Here: The last received VehicleStateList, with is_valid = false if no msg was received.
     * \param inputs Inputs given by the calling Matlab script. Here: Optional Matlab Domain ID, Optional uint32 specifying the milliseconds to max. wait for a msg
     */
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {   
        //Required for data creation
        matlab::data::ArrayFactory factory;

        //Check if inputs match the expected input
        checkArguments(outputs, inputs);

        //Get domain_id from input, if set
        auto domain_id = 1; //DEFAULT
        if(inputs.size() >= 1)
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
            reader = std::make_shared<cpm::ReaderAbstract<VehicleStateListPubSubType>>(
                participant->get_participant(), 
                "vehicleStateList", 
                false, 
                false, 
                false
            );
        }

        //Create output struct
        matlab::data::StructArray state_list_object = factory.createStructArray(
            {1, 1},
            {"t_now", 
            "period_ms", 
            "state_list",
            "vehicle_observation_list",
            "active_vehicle_ids",
            "is_valid"});

        //Default: No data was received, output is invalid
        state_list_object[0]["is_valid"] = factory.createScalar<bool>(false);

        //Debugging: Print msg type
        // matlabPtr->feval(u"disp", 0, 
        //             std::vector<matlab::data::Array>({ factory.createScalar(type_->getName()) }));

        //Check if the reader should wait wait_time_ms milliseconds for incoming messages
        if (inputs.size() >= 2) {
            matlab::data::TypedArray<uint32_t> wait_time_ms = std::move(inputs[1]);
            reader->wait_for_unread_message(wait_time_ms[0]);
        }

        
        auto samples = reader->take();

        if (samples.size() > 0)
        {
            auto vehicle_state_list = *(samples.rbegin());

            matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("Received something") }));

                matlabPtr->feval(u"disp", 0, 
                    std::vector<matlab::data::Array>({ factory.createScalar("Working on it") }));
                //Now, vehicle_state_list contains all relevant data
                //First, start with the data that is easy to copy

                //We received something, so set is_valid to true
                state_list_object[0]["is_valid"] = factory.createScalar<bool>(true);

                //Set everything that is not a list
                state_list_object[0]["t_now"] = factory.createScalar<uint64_t>(vehicle_state_list.t_now());
                state_list_object[0]["period_ms"] = factory.createScalar<uint64_t>(vehicle_state_list.period_ms());

                //Set lists
                write_active_vehicle_ids(state_list_object, vehicle_state_list);
                write_vehicle_state_list(state_list_object, vehicle_state_list);
                write_vehicle_observation_list(state_list_object, vehicle_state_list);
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

        if (inputs.size() > 2)
        {
            matlabPtr->feval(u"error", 0, 
                std::vector<matlab::data::Array>({ factory.createScalar("Wrong input size. Optional inputs are (in this order): Domain ID, max. time in ms. to wait for a message!") }));
        }

        //Test for optional unsigned int parameter for timeout / max. wait time
        if (inputs.size() >= 2)
        {
            if (inputs[1].getType() != matlab::data::ArrayType::UINT32)
            {
                matlabPtr->feval(u"error", 0, 
                    std::vector<matlab::data::Array>({ 
                        factory.createScalar("The second optional input must be of type uint32. It specifies how long the reader should wait for a message, in milliseconds (default: don't wait).") 
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
                        factory.createScalar("The optional input domain_id must be of type uint32. It specifies the domain ID of the participant (default is 1).") 
                }));
            }
        }
    }

    /**
     * \brief Writes the received active_vehicle_ids data to the matlab object
     * \param state_list_object The matlab object
     * \param vehicle_state_list The DDS object
     */
    void write_active_vehicle_ids(matlab::data::StructArray &state_list_object, VehicleStateList &vehicle_state_list)
    {
        matlab::data::ArrayFactory factory;

        matlab::data::TypedArray<int32_t> active_ids = factory.createArray<int32_t>({1, vehicle_state_list.active_vehicle_ids().size()});
        for (auto i = 0; i < vehicle_state_list.active_vehicle_ids().size(); ++i)
        {
            active_ids[i] = vehicle_state_list.active_vehicle_ids().at(i);
        }

        state_list_object[0]["active_vehicle_ids"] = active_ids;
    }

    /**
     * \brief Writes the received state_list data to the matlab object
     * \param state_list_object The matlab object
     * \param vehicle_state_list The DDS object
     */
    void write_vehicle_state_list(matlab::data::StructArray &state_list_object, VehicleStateList &vehicle_state_list)
    {
        matlab::data::ArrayFactory factory;

        //Create VehicleState Array
        matlab::data::StructArray vehicle_state_array = factory.createStructArray(
            {1, vehicle_state_list.state_list().size()},
            {
                "vehicle_id",
                "create_stamp",
                "valid_after_stamp",
                "pose_x",
                "pose_y",
                "pose_yaw",
                "IPS_update_age_nanoseconds",
                "odometer_distance",
                "imu_acceleration_forward",
                "imu_acceleration_left",
                "imu_acceleration_up",
                "imu_yaw",
                "imu_yaw_rate",
                "speed",
                "battery_voltage",
                "motor_current",
                "motor_throttle",
                "steering_servo",
                "is_real"
            }
        );

        for (auto i = 0; i < vehicle_state_list.state_list().size(); ++i) {
            //Get vehicle_data
            auto vehicle_data = vehicle_state_list.state_list().at(i);

            //Set data values
            vehicle_state_array[i]["vehicle_id"] = factory.createScalar<uint8_t>(vehicle_data.vehicle_id());
            vehicle_state_array[i]["create_stamp"] = factory.createScalar<uint64_t>(vehicle_data.header().create_stamp().nanoseconds());
            vehicle_state_array[i]["valid_after_stamp"] = factory.createScalar<uint64_t>(vehicle_data.header().valid_after_stamp().nanoseconds());
            vehicle_state_array[i]["IPS_update_age_nanoseconds"] = factory.createScalar<uint64_t>(vehicle_data.IPS_update_age_nanoseconds());
            vehicle_state_array[i]["odometer_distance"] = factory.createScalar<double>(vehicle_data.odometer_distance());
            vehicle_state_array[i]["imu_acceleration_forward"] = factory.createScalar<double>(vehicle_data.imu_acceleration_forward());
            vehicle_state_array[i]["imu_acceleration_left"] = factory.createScalar<double>(vehicle_data.imu_acceleration_left());
            vehicle_state_array[i]["imu_acceleration_up"] = factory.createScalar<double>(vehicle_data.imu_acceleration_up());
            vehicle_state_array[i]["imu_yaw"] = factory.createScalar<double>(vehicle_data.imu_yaw());
            vehicle_state_array[i]["imu_yaw_rate"] = factory.createScalar<double>(vehicle_data.imu_yaw_rate());
            vehicle_state_array[i]["speed"] = factory.createScalar<double>(vehicle_data.speed());
            vehicle_state_array[i]["battery_voltage"] = factory.createScalar<double>(vehicle_data.battery_voltage());
            vehicle_state_array[i]["motor_current"] = factory.createScalar<double>(vehicle_data.motor_current());
            vehicle_state_array[i]["motor_throttle"] = factory.createScalar<double>(vehicle_data.motor_throttle());
            vehicle_state_array[i]["steering_servo"] = factory.createScalar<double>(vehicle_data.steering_servo());
            vehicle_state_array[i]["is_real"] = factory.createScalar<bool>(vehicle_data.is_real());

            //Special case: Pose, currently not handled as separate object (might get annoying for the user)
            vehicle_state_array[i]["pose_x"] = factory.createScalar<double>(vehicle_data.pose().x());
            vehicle_state_array[i]["pose_y"] = factory.createScalar<double>(vehicle_data.pose().y());
            vehicle_state_array[i]["pose_yaw"] = factory.createScalar<double>(vehicle_data.pose().yaw());
        }

        state_list_object[0]["state_list"] = vehicle_state_array;
    }

    /**
     * \brief Writes the received vehicle_observation_list data to the matlab object
     * \param state_list_object The matlab object
     * \param vehicle_state_list The DDS object
     */
    void write_vehicle_observation_list(matlab::data::StructArray &state_list_object, VehicleStateList &vehicle_state_list)
    {
        matlab::data::ArrayFactory factory;

        //Create VehicleState Array
        matlab::data::StructArray vehicle_observation_array = factory.createStructArray(
            {1, vehicle_state_list.vehicle_observation_list().size()},
            {
                "vehicle_id",
                "create_stamp",
                "valid_after_stamp",
                "pose_x",
                "pose_y",
                "pose_yaw"
            }
        );

        for (auto i = 0; i < vehicle_state_list.vehicle_observation_list().size(); ++i) {
            //Get vehicle_data
            auto vehicle_data = vehicle_state_list.vehicle_observation_list().at(i);

            //Set data values
            vehicle_observation_array[i]["vehicle_id"] = factory.createScalar<uint8_t>(vehicle_data.vehicle_id());
            vehicle_observation_array[i]["create_stamp"] = factory.createScalar<uint64_t>(vehicle_data.header().create_stamp().nanoseconds());
            vehicle_observation_array[i]["valid_after_stamp"] = factory.createScalar<uint64_t>(vehicle_data.header().valid_after_stamp().nanoseconds());

            //Special case: Pose, currently not handled as separate object (might get annoying for the user)
            vehicle_observation_array[i]["pose_x"] = factory.createScalar<double>(vehicle_data.pose().x());
            vehicle_observation_array[i]["pose_y"] = factory.createScalar<double>(vehicle_data.pose().y());
            vehicle_observation_array[i]["pose_yaw"] = factory.createScalar<double>(vehicle_data.pose().yaw());
        }

        state_list_object[0]["vehicle_observation_list"] = vehicle_observation_array;
    }
};