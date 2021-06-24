#!/bin/bash
# Can be used standalone, but also gets called by install.sh

if ! [ $(id -u) = 0 ]; then
   echo "This script needs super user privileges. Try to run it again with sudo." >&2
   exit 1
fi
if [ $SUDO_USER ]; then
    real_user=$SUDO_USER
else
    real_user=$(whoami)
fi

# Install update-alternatives for GCC for MEX compilation (newer versions than v9 are not supported by Matlab as of June 2021)
sudo apt install gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 10
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 10
# Note: Setting the compiler to gcc-9 etc. is done in the MEX-build script, although that needs sudo as well, 
# so that the system setup of the user does not get modified too much

# Get the path to your Matlab executable (does not work without -i, need user because PATH is not set for sudo)
MATLAB_PATH=$(sudo -i -u $real_user which matlab; exit)
MATLAB_PATH=${MATLAB_PATH//'//'/'/'} # Get rid of double-// in the path, which may occur when using sudo -i
if [[ -z "${MATLAB_PATH// }" ]]
then
    echo "WARNING: MATLAB is not installed / on your path. If you install it later on, you must run this script (matlab_setup.sh) manually again!"
else
    # Adapt the path to be the path to matlab (cut away /bin/matlab), then expand it to point to the path of the lib to replace
    MATLAB_PATH=${MATLAB_PATH%/*}
    MATLAB_PATH=${MATLAB_PATH%/*}
    MATLAB_LIBSTDCPP_PATH="$MATLAB_PATH/sys/os/glnxa64"

    echo "Now going into $MATLAB_LIBSTDCPP_PATH to replace Matlab's libstdc++ file with the system version..."
    cd "$MATLAB_LIBSTDCPP_PATH"
    sudo mkdir -p old
    sudo mv libstdc++.so.6* old
    sudo ln -s /usr/lib/x86_64-linux-gnu/libstdc++.so.6 libstdc++.so.6 
    echo "Done"
fi