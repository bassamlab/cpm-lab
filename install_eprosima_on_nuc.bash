#!/bin/bash
#Get command line arguments
for i in "$@"
do
case $i in
    -vi=*|--vehicle_ids=*)
    vehicle_ids="${i#*=}"
    shift # past argument=value
    ;;
    -va=*|--vehicle_amount=*)
    vehicle_amount="${i#*=}"
    shift # past argument=value
    ;;
    *)
          # unknown option
    ;;
esac
done

password=''
read -s -p 'Enter the sudo password for the NUCs: ' password
echo ''

#Check for existence of required command line arguments
if ( [ -z "$vehicle_ids" ] ) || [ -z "$password" ]
then
      echo "Invalid use, enter vehicle IDs or set a password in the script"
      echo "The password was not permanently stored in this file due to it being uploaded to Git"
      echo "Also, providing it as an argument in the command line would mean that the password is readable in the command line log"
      exit 1
fi

# Path to replace the MATLAB executable for C++ by the system version to avoid problems with the generated MEX files
MATLAB_LIBSTDCPP_PATH="/opt/MATLAB/R2020a/sys/os/glnxa64"

IFS=,
for val in $vehicle_ids;
do
    ip=$(printf "192.168.1.2%02d" ${val})
    echo $ip
    sshpass -p $password ssh -t controller@$ip << EOF
        echo "${password}" | sudo -S apt-get update;sudo apt-get install cmake g++ python3-pip wget git libasio-dev libtinyxml2-dev libssl-dev -y
        cd ~
        mkdir -p eprosima_files
        cd eprosima_files

        git clone https://github.com/eProsima/foonathan_memory_vendor.git
        cd foonathan_memory_vendor
        sudo git clean -dfx
        git checkout 0be8f746ee33b3366528d2533087754ce14732b0
        mkdir -p build
        cd build
        cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/ -DBUILD_SHARED_LIBS=ON
        sudo cmake --build . --target install

        cd ~/eprosima_files
        git clone https://github.com/eProsima/Fast-CDR.git
        cd Fast-CDR
        sudo git clean -dfx
        git checkout cff6ea98f66d5fd4d53541e183676257a42a6c23
        mkdir -p build
        cd build
        cmake .. 
        sudo cmake --build . --target install

        cd ~/eprosima_files
        git clone https://github.com/eProsima/Fast-DDS.git
        cd Fast-DDS
        sudo git clean -dfx
        git checkout 5e96b4f5044ef1fc2932b9b622db48dab299d0c2
        mkdir -p build
        cd build
        cmake .. 
        sudo cmake --build . --target install

        echo "Now going into ${MATLAB_LIBSTDCPP_PATH} to replace Matlab's libstdc++ file with the system version..."
        cd "${MATLAB_LIBSTDCPP_PATH}"
        sudo mkdir -p old
        sudo mv libstdc++.so.6* old
        sudo ln -s /usr/lib/x86_64-linux-gnu/libstdc++.so.6 libstdc++.so.6 
        echo "Done"

        sudo reboot
EOF
done