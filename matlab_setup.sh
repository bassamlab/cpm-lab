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

# Get the path to your Matlab executable (does not work without -i, need user because PATH is not set for sudo)
MATLAB_PATH=$(sudo -i -u $real_user which matlab; exit)
MATLAB_PATH=${MATLAB_PATH//'//'/'/'} # Get rid of double-// in the path, which may occur when using sudo -i
if [[ -z "${MATLAB_PATH// }" ]]
then
    echo "Please install Matlab and add it to your PATH before continuing"
    echo "Aborting..."
    exit 1
fi

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