#!/bin/bash
# This scripts automates the mandatory steps in order to compile the cpm
# software suit (/software/build_all.bash). As it installs build tools it needs
# super user privileges (e.g. sudo)
#
# This script will download various files and therefor creates its own
# directory 'tmp' relative to this scripts ablsoute path. 
#
# This script is compatible with Debian and RedHat based distirbutions but has
# been tested specifically with Ubuntu 18.04.3 LTS and Fedora 31.
#
# NOTE: This script requires user input in step
# - '3.2 Installation'
#  -- to click mannually through the RTI Connext DDS 6.0.0 GUI installer
#  -- to provide an absolute path to an RTI license file
# - '3.3 Environment Setup' to enter a unique DDS Domain
# - Default Arguments may be passed via commandline ./setup_cpm_build_environment.sh path_to_rti_license domain_id

# This causes the bash script to return non-zero exit code as soon a command fails
set -e
# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command failed with exit code $?."' EXIT

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

### 0. Preconditioning #########################################################



## 0.1 Check for Super User Privileges and Handle User Rights
# ref: https://askubuntu.com/a/30157/8698
if ! [ $(id -u) = 0 ]; then
   echo "This script needs super user privileges. Try to run it again with sudo." >&2
   exit 1
fi
if [ $SUDO_USER ]; then
    real_user=$SUDO_USER
else
    real_user=$(whoami)
fi
RU_HOME=$( getent passwd $real_user | cut -d: -f6 )

## 0.2 Determine OS & Set Commands Accordingly
if [[ "$(awk -F= '/^NAME/{print $2}' /etc/os-release)" != '"Ubuntu"' ]]; then
    echo "You aren't using Ubuntu. Please use Ubuntu 18.04!"
    exit 1;
    if [[ "$(awk -F= '/^VERSION_ID/{print $2}' /etc/os-release)" != '"18.04"' ]]; then
        echo "You are using the wrong version. Please switch to Ubuntu 18.04. Otherwise stability is not guaranteed.";
        # exit 1;        
    fi
fi

### 0.4 Parse Command Line Arguments
CI=0
DOMAIN_ID="NONE"
# SIMULATION is not set and thus lab mode is the default
while [[ $# -gt 0 ]] && [[ "$1" == "--"* ]] ;
do
    opt="$1";
    shift;              #expose next argument
    case "$opt" in
        "--" ) break 2;;
        "--ci" )
           CI="1";;
        "--domain_id="* )
           DOMAIN_ID="${opt#*=}";;
        "--license_path="* )
           LICENSE_PATH="${opt#*=}";;
        "--simulation" )
           SIMULATION="1";;     #set to some default value
        "--rti_installer_automation_path="* )
           RTI_INSTALLER_AUTOMATION_PATH="${opt#*=}";;
        *) echo >&2 "Invalid option: $@"; exit 1;;
   esac
done

if [ "$DOMAIN_ID" == "NONE" ]; then
    read -p 'Please provide a integer Domain ID: ' DOMAIN_ID
    while [ -z "$DOMAIN_ID" ]; do
          echo "No domain id was specified"
          read DOMAIN_ID
    done

    if ! [[ "$DOMAIN_ID" =~ ^[0-9]+$ ]]
    then
        echo "please provide integer number for domain ID"
        exit 1
    fi
fi

echo "CI =" $CI
echo "Domain ID =" $DOMAIN_ID
if [ -z $SIMULATION ]; then
    echo "Simulation =" $SIMULATION
else
    echo "Simulation only mode is disabled"
fi


### 0.5 Create folders for nuc and raspberry packages
if [ -z $SIMULATION ]; then
    sudo mkdir -p "/var/www/html/nuc"
    sudo chmod a+rwx "/var/www/html/nuc"
    sudo mkdir -p "/var/www/html/raspberry"
    sudo chmod a+rwx "/var/www/html/raspberry"
fi

### 0.6 Create temporary folder
sudo -u ${real_user} mkdir -p tmp

### 1. Ubuntu & Packages #######################################################
PM="apt"
UPDATE="update && apt upgrade -y"
BUILD_ESSENTIALS="install build-essential -y"
BUILD_TOOLS="install iproute2 git libasio-dev libtinyxml2-dev libssl-dev tmux cmake libgtkmm-3.0-dev libxml++2.6-dev ntp jstest-gtk openssh-client openssh-server sshpass -y"
DEP_NO_SIM="install apache2 libgstreamer1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio -y"
OPENJDK="install openjdk-11-jdk -y"
PYLON_URL="https://www.baslerweb.com/fp-1523350893/media/downloads/software/pylon_software/pylon_5.0.12.11829-deb0_amd64.deb"
eval "${PM}" "${UPDATE}"
eval "${PM}" "${BUILD_ESSENTIALS}"
eval "${PM}" "${BUILD_TOOLS}"
eval "${PM}" "${OPENJDK}"
if [ $SIMULATION == 0 ]; then
    eval "${PM}" "${DEP_NO_SIM}"
fi

### 1.1 CMake #######################################################
# Download, verify and install a newer CMake version than distributed in Ubuntu 18.04
if [ -d "./cmake_tmp" ]; then rm -rf ./cmake_tmp; fi
mkdir cmake_tmp
cd cmake_tmp

curl -OL https://github.com/Kitware/CMake/releases/download/v3.22.1/cmake-3.22.1-SHA-256.txt
curl -OL https://github.com/Kitware/CMake/releases/download/v3.22.1/cmake-3.22.1.tar.gz
sha256sum -c --ignore-missing cmake-3.22.1-SHA-256.txt

if [ ! $? -eq 0 ];
then
    echo "The CMake SHA256 is not valid, aborting..." >&2
    exit 1
fi

curl -OL https://github.com/Kitware/CMake/releases/download/v3.22.1/cmake-3.22.1-SHA-256.txt.asc
gpg --keyserver hkps://keyserver.ubuntu.com --recv-keys C6C265324BBEBDC350B513D02D2CEF1034921684
gpg --verify cmake-3.22.1-SHA-256.txt.asc cmake-3.22.1-SHA-256.txt

if [ ! $? -eq 0 ];
then
    echo "The CMake GPG for SHA256 is not valid, aborting..." >&2
    exit 1
fi

# Install CMake
tar -xvzf cmake-3.22.1.tar.gz
cd cmake-3.22.1
cmake .
make
sudo make install

# Get rid of tmp cmake folder
cd ..
cd ..
rm -rf ./cmake_tmp

### 2. Joystick / Gamepad ######################################################
#With a Joystick or a Gamepad you can drive vehicles manually in the Lab Control Center (LCC)
apt install jstest-gtk 


### 3. Eprosima #################################################################
## 3.1 Init submodules and / or update them (in case of version change)
## Clean old cmake files that caused problems when re-building with a newer eProsima verions
## Also: Use sudo because some of the files have been created with sudo privileges (e.g. eProsima to be able to install it globally using sudo cmake --build . --target install)
cd "$DIR"
sudo git clean -xfd
sudo git submodule foreach --recursive git clean -xfd
sudo git reset --hard
sudo git submodule foreach --recursive git reset --hard
sudo git submodule update --init --recursive

# As sudo is used here, chown is used later on to make sure that the user can call build_all.bash
# Needs to be done before and after the calls for eProsima building, as not all of them use sudo commands
cd "$DIR/"
sudo chown -R $real_user ./

## To make sure that no outdated eProsima version is used, delete the compile .so files
cd /usr/local/lib/
sudo rm ./libfastcdr* || true # Makes sure that the script does not stop if the files does not exist without using -f
sudo rm ./libfastrtps* || true # Makes sure that the script does not stop if the files does not exist without using -f
sudo rm ./libfoonathan* || true # Makes sure that the script does not stop if the files does not exist without using -f
sudo rm -rf ./foonathan_memory/ || true # Makes sure that the script does not stop if the files does not exist without using -f
sudo rm -rf ./cmake/fastcdr/ || true # Makes sure that the script does not stop if the files does not exist without using -f

## 3.2 Install FastDDS (system-wide, thus flags changed as specified in the note on the eProsima website)
### 3.2.1 Install Foonathan memory vendor
cd "$DIR/cpm_lib/thirdparty/"
cd foonathan_memory_vendor
mkdir -p ./build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/ -DBUILD_SHARED_LIBS=ON
sudo cmake --build . --target install

### 3.2.2 Install Fast-CDR
cd "$DIR/cpm_lib/thirdparty/"
cd Fast-CDR
mkdir -p ./build
cd build
cmake ..
sudo cmake --build . --target install

### 3.2.3 Install Fast-DDS
cd "$DIR/cpm_lib/thirdparty/"
cd Fast-DDS
mkdir -p ./build
cd build
cmake ..
sudo cmake --build . --target install

## 3.3 Install FastDDS-Gen
# Regarding Gradle: apt packages are "too new" for the currently used FastDDS-Gen, deprecated commands lead to build failure
# Thus: Use the provided gradle script for a temporary download & install of Gradle
cd "$DIR/cpm_lib/thirdparty/"
cd Fast-DDS-Gen
sudo ./gradlew assemble # TODO: Fails without sudo, but is this safe enough? Gradle is always downloaded and then gets sudo priviliges

# Select a unique DDS domain! To avoid interference from other users in the same
# network, you need to set a DDS domain ID that is different from everyone in
# the network. The domain ID is assumed to be in the environment variable
# DDS_DOMAIN.

echo "export DDS_DOMAIN=""${DOMAIN_ID}" > /etc/profile.d/rti_connext_dds.sh
echo "export RASPBIAN_TOOLCHAIN=/opt/cross-pi-gcc" >> /etc/profile.d/rti_connext_dds.sh
echo "export RPIPWD=cpmcpmcpm" >> /etc/profile.d/rti_connext_dds.sh
# Reboot or source to apply the changes made to the environment variables.
source /etc/profile.d/rti_connext_dds.sh

## 3.4 Install eProsima ARM libraries
# only needed in real lab mode
################## TODO ???????????????????????????????????????? ############################

## 3.5 Adapt the current GCC version - install a version compatible with Matlab (to create Mex Files)
# This version can later be selected using update-alternatives

## 3.6 Adapt the Matlab installation: Replace the libstdc++ file with a link to the system's file
# Get the executable path
cd "$DIR/"
sudo bash ./matlab_setup.sh

## 3.7 Also: Make sure that the right user owns these files, so that they can be changed without requiring sudo for everything
# If this is not done, build_all etc. will not work without using sudo
# Needs to be done before and after the calls for eProsima building, as not all of them use sudo commands
cd "$DIR/"
sudo chown -R $real_user ./

### 4. Indoor Positioning System (Setup) #######################################
# The Indoor Positioning System depends on the camera software Basler Pylon and
# on OpenCV 4.0.0.
# https://cpm.embedded.rwth-aachen.de/doc/display/CLD/Indoor+Positioning+System

if [ -z $SIMULATION ]; then
    ## 4.1 OpenCV 4.0.0
    apt install openjdk-11-jdk -y
    cd /tmp
    sudo -u $real_user git clone https://github.com/opencv/opencv.git
    cd ./opencv
    sudo -u $real_user git checkout 4.0.0
    sudo -u $real_user mkdir ./build
    cd ./build
    sudo -u $real_user cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/opt/opencv400 ..
    N=$(nproc)
    sudo -u $real_user make -j$N
    make install
    cd "$DIR"
    if [ ! -d "/opt/opencv400/lib" ]; then
        ln -s /opt/opencv400/lib64 /opt/opencv400/lib
    fi
    rm -rf /tmp/opencv

    ## 4.2 Basler Pylon 5
    cd "$DIR/tmp"
    sudo -u $real_user wget https://www.baslerweb.com/fp-1523350893/media/downloads/software/pylon_software/pylon_5.0.12.11829-deb0_amd64.deb
    dpkg -i pylon*.deb
fi

rm -rf "${DIR}/tmp"

### 5. Inform user about success and next steps ################################
echo "Success! Ready to build the cpm software suite."
echo "Reboot your PC or execute 'source /etc/profile.d/rti_connext_dds.sh'"
echo "Then execute './build_all.bash' or './build_all.bash --simulation'"

exit 0
