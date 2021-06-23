#!/bin/bash
if ! [ $(id -u) = 0 ]; then
   echo "This script needs super user privileges. Try to run it again with sudo." >&2
   exit 1
fi
if [ $SUDO_USER ]; then
    real_user=$SUDO_USER
else
    real_user=$(whoami)
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
CPM_DIR=${DIR%/*}

# Switch to gcc-9, which is the newest version supported by Matlab as of June 2021
# This only works if you have called the matlab_setup script before!
GCC_ALTERNATIVES=$(update-alternatives --list gcc)
GPP_ALTERNATIVES=$(update-alternatives --list g++)
if [[ "$GCC_ALTERNATIVES" != *"gcc-9"* ]] || [[ "$GPP_ALTERNATIVES" != *"g++-9"* ]]
then
    echo "ERROR: Missing update-alternatives. Please call matlab_setup.sh before installing the MEX Bindings for Matlab. You can find the script in the top folder of the repository."
    echo "Aborting..."
    exit 1
fi
sudo update-alternatives --set gcc /usr/bin/gcc-9
sudo update-alternatives --set g++ /usr/bin/g++-9

# Get the path to MEX, which is most often not known to the sudo user
MEX_PATH=$(sudo -i -u $real_user which mex; exit)

# Check if MEX is installed
if [[ -z "${MEX_PATH// }" ]]; then
    echo "ERROR: Please install Matlab and add it (and mex, also in Matlabs bin folder) to your PATH before continuing"
    echo "Aborting..."
    exit 1
fi

# Compile the MEX files
echo "Now compiling your MEX files, given that mex is on the PATH and the cpm lib, FastDDS and FastCDR are installed..."
cd $DIR
$MEX_PATH system_trigger_reader.cpp -L"$CPM_DIR/build/" -lcpm -I"$CPM_DIR/include/" -L/usr/local/lib -lfastcdr -lfastrtps
$MEX_PATH vehicle_state_list_reader.cpp -L"$CPM_DIR/build/" -lcpm -I"$CPM_DIR/include/" -L/usr/local/lib -lfastcdr -lfastrtps
$MEX_PATH ready_status_writer.cpp -L"$CPM_DIR/build/" -lcpm -I"$CPM_DIR/include/" -L/usr/local/lib -lfastcdr -lfastrtps
$MEX_PATH vehicle_command_trajectory_writer.cpp -L"$CPM_DIR/build/" -lcpm -I"$CPM_DIR/include/" -L/usr/local/lib -lfastcdr -lfastrtps
echo "Done"

# Revert to your system's default gcc
sudo update-alternatives --auto gcc
sudo update-alternatives --auto g++