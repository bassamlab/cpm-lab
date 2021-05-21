#include "mex.h"

//The Matlab docs specify how to integrate external libraries (.so, with -l in mex call)
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
    mexPrintf("Starting mex test\n");

    //Statically hold the cpm DataWriter, use mexLock to keep this (unlock to delete)
    static std::shared_ptr<cpm::Writer<VisualizationPubSubType>> writer_ptr = std::make_shared<cpm::Writer<VisualizationPubSubType>>("visualization");

    mexPrintf("Object constructed\n");
    
    //Get input command
    auto input_buf = mxArrayToString(prhs[0]);
    std::string string_input(input_buf);
    mxFree(input_buf);

    mexPrintf("Input read\n");

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
        writer_ptr->write(msg);
    }

    mexPrintf("Finished\n");
}