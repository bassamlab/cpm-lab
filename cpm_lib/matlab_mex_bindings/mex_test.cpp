#include "mex.h"

//The Matlab docs specify how to integrate external libraries (.so, with -l in mex call)
#include "cpm/init.hpp"
#include "cpm/Logging.hpp"
#include "cpm/Participant.hpp"
#include "cpm/Writer.hpp"
#include "cpm/dds/VisualizationPubSubTypes.h"

//Standard libraries
#include <memory>
#include <string>

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

    // static bool had_init = false;
    // if (! had_init)
    // {
    //     char program_name[] = "matlab";
    //     char arg1[] = "--dds_domain=12";
    //     char arg2[] = "--logging_id=matlab_test";
    //     char *argv[] = {program_name, arg1, arg2};
    //     int argc = 3;

    //     cpm::init(argc, argv);
    // }
    // had_init = true;

    //Statically hold the cpm DataWriter, use mexLock to keep this (unlock to delete)
    static std::shared_ptr<cpm::Participant> particpant_ptr = std::make_shared<cpm::Participant>(1);
    static std::shared_ptr<cpm::Writer<VisualizationPubSubType>> writer_ptr = std::make_shared<cpm::Writer<VisualizationPubSubType>>(particpant_ptr->get_participant(), "visualization");
    
    //Get input command
    auto input_buf = mxArrayToString(prhs[0]);
    std::string string_input(input_buf);
    mxFree(input_buf);

    if (string_input.compare("create"))
    {
        mexLock();
    }
    else if (string_input.compare("delete"))
    {
        mexUnlock();
    }
    else if (string_input.compare("write"))
    {
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
        writer_ptr->write(msg);
    }
    else if (string_input.compare("log"))
    {
        cpm::Logging::Instance().write("%s", "TEST");
    }

    mexPrintf("\nfinished\n");
}