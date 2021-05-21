#include "mex.h"

//The Matlab docs specify how to integrate external libraries (.so, with -l in mex call)
#include "cpm/dds/VisualizationPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

//Standard libraries
#include <memory>
#include <string>
#include <unistd.h>

//Kompilieren lässt sich die Datei zurzeit wie folgt: (Pfade bitte entsprechend anpassen)
// /usr/local/MATLAB/R2021a/bin/mex mex_test.cpp -L/home/leon/dev/software/cpm_lib/build/ -lcpm -I/home/leon/dev/software/cpm_lib/include -L/usr/local/lib -lfastcdr -lfastrtps

/**
 * \brief The mex function called by matlab. This only handles C++ objects / how to keep the alive, and tests an eProsima writer accordingly.
 * It provides no other use yet (e.g. passing messages / receiving them)
 * \param nlhs Output, number, not used here
 * \param plhs Output, not used here
 * \param nrhs Input, not used here
 * \param prhs Input, command
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    mexPrintf("\nStarting mex test...\n");

    static eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    static eprosima::fastdds::dds::Publisher* publisher_ = nullptr;
    static eprosima::fastdds::dds::Topic* topic_ = nullptr;
    static eprosima::fastdds::dds::DataWriter* writer_ = nullptr;
    static eprosima::fastdds::dds::TypeSupport type_(new VisualizationPubSubType());
    
    //Get input command
    auto input_buf = mxArrayToString(prhs[0]);
    mexPrintf(input_buf);
    std::string string_input(input_buf);
    mxFree(input_buf);

    if (string_input.compare("create") == 0)
    {
        mexPrintf("\nCreating...\n");

        participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
            0, 
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->get_default_participant_qos()
        );

        if (participant_ == nullptr)
        {
            mexPrintf("\nERROR\n");
            return;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the publications Topic
        topic_ = participant_->create_topic("visualization", type_->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            mexPrintf("\nERROR\n");
            return;
        }

        // Create the Publisher
        publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);

        if (publisher_ == nullptr)
        {
            mexPrintf("\nERROR\n");
            return;
        }

        // Create the DataWriter
        writer_ = publisher_->create_datawriter(topic_, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT, nullptr);

        if (writer_ == nullptr)
        {
            mexPrintf("\nERROR\n");
            return;
        }

        usleep(500000);
        
        mexLock();
    }
    else if (string_input.compare("delete") == 0)
    {
        mexPrintf("\nDeleting...\n");

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

        mexUnlock();
    }
    else if (string_input.compare("write") == 0)
    {
        mexPrintf("\nWriting...\n");

        Visualization msg;
        msg.id(3);
        msg.type(VisualizationType::StringMessage);
        msg.time_to_live(10000000000);
        msg.size(1.0);

        Point2D point1_3;
        point1_3.x(0.2);
        point1_3.y(0.2);
        
        std::vector<Point2D> msg_points {point1_3};
        msg.points(msg_points);

        Color msg_color;
        msg_color.a(255);
        msg_color.r(255);
        msg_color.g(255);
        msg_color.b(0);
        msg.color(msg_color);

        msg.string_message("Hello LCC!");

        if (writer_ == nullptr)
        {
            mexPrintf("\nERROR\n");
            return;
        }

        auto id = static_cast<int>(writer_->get_publisher()->get_participant()->get_domain_id());
        mexPrintf("Domain ID is: %i", id);

        writer_->write(&msg);

        usleep(500000);
    }

    mexPrintf("\nfinished\n");
}