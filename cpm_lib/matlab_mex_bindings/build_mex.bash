#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
CPM_DIR=${DIR%/*}

# Get the path to MEX
MEX_PATH_DEFAULT="/opt/MATLAB/R2019b/bin/mex"

echo "The default Matlab path is ${MEX_PATH_DEFAULT}"
read -p 'To compile the library for Matlab, enter the path to mex or nothing for default: ' MEX_PATH
#check, if a path to Matlab was entered
if [ -z "$MEX_PATH" ]
then
    echo "Using the default path ${MEX_PATH_DEFAULT}"
    MEX_PATH="${MEX_PATH_DEFAULT}"
fi

# Check if the file exists
if ! [[ -f "$MEX_PATH" ]]
then
    echo "The file / path you entered was invalid, skipping compilation..."
fi

# OUTDATED: Update-alternatives is not flexible w.r.t. Ubuntu and Matlab versions (would need different GCC versions)
# Check for sudo permissions
# if ! sudo true; then
#     warning_msg="WARNING: Without sudo, the Mex Matlab Bindings will not be built! "
#     warning_msg+="You may build them on your own (see build_mex.bash or README.md in matlab_mex_bindings / Confluence Doc. to see how) without using sudo, "
#     warning_msg+="which may mean that you cannot call update-alternatives to switch to a supported GCC version. "
#     warning_msg+="This may not actually be problematic, as MEX will only print a warning in that case."
#     echo $warning_msg
#     exit 0
# fi

# # Switch to gcc-9, which is the newest version supported by Matlab as of June 2021
# # This only works if you have called the matlab_setup script before!
# GCC_ALTERNATIVES=$(update-alternatives --list gcc)
# GPP_ALTERNATIVES=$(update-alternatives --list g++)
# if [[ "$GCC_ALTERNATIVES" != *"gcc-9"* ]] || [[ "$GPP_ALTERNATIVES" != *"g++-9"* ]]
# then
#     echo "ERROR: Missing update-alternatives. Please call matlab_setup.sh before installing the MEX Bindings for Matlab. You can find the script in the top folder of the repository."
#     echo "Aborting..."
#     exit 0
# fi

# # Checks done, now start
# sudo update-alternatives --set gcc /usr/bin/gcc-9
# sudo update-alternatives --set g++ /usr/bin/g++-9

# Compile the MEX files
if (! [[ -z "$MEX_PATH" ]]) && [ -f "$MEX_PATH" ]
then
    echo "Now compiling your MEX files, given that mex is on the PATH and the cpm lib, FastDDS and FastCDR are installed..."
    cd $DIR
    $MEX_PATH system_trigger_reader.cpp -L"$CPM_DIR/build/" -lcpm -I"$CPM_DIR/include/" -L/usr/local/lib -lfastcdr -lfastrtps
    $MEX_PATH vehicle_state_list_reader.cpp -L"$CPM_DIR/build/" -lcpm -I"$CPM_DIR/include/" -L/usr/local/lib -lfastcdr -lfastrtps
    $MEX_PATH ready_status_writer.cpp -L"$CPM_DIR/build/" -lcpm -I"$CPM_DIR/include/" -L/usr/local/lib -lfastcdr -lfastrtps
    $MEX_PATH vehicle_command_trajectory_writer.cpp -L"$CPM_DIR/build/" -lcpm -I"$CPM_DIR/include/" -L/usr/local/lib -lfastcdr -lfastrtps
    $MEX_PATH vehicle_command_speed_curvature_writer.cpp -L"$CPM_DIR/build/" -lcpm -I"$CPM_DIR/include/" -L/usr/local/lib -lfastcdr -lfastrtps
    $MEX_PATH vehicle_command_direct_writer.cpp -L"$CPM_DIR/build/" -lcpm -I"$CPM_DIR/include/" -L/usr/local/lib -lfastcdr -lfastrtps
    echo "Done"
fi

# Revert to your system's default gcc
# sudo update-alternatives --auto gcc
# sudo update-alternatives --auto g++