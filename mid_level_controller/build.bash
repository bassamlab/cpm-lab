#!/bin/bash
# exit when any command fails
set -e

mkdir -p build_arm
mkdir -p build_arm_sim
mkdir -p build_x64_sim

# Build for simulation on desktop
pushd build_x64_sim
cmake .. -DBUILD_ARM=OFF -DBUILD_SIMULATION=ON
make -j$(nproc)
popd

if [ -z $SIMULATION ]; then

    export RASPBIAN_TOOLCHAIN=/opt/cross-pi-gcc
    # Build for simulation on Raspberry
    pushd build_arm_sim
    cmake .. -DBUILD_ARM=ON -DBUILD_SIMULATION=ON -DCMAKE_TOOLCHAIN_FILE=../Toolchain.cmake
    make -j$(nproc)
    cp -R ../package/ .
    cp vehicle_rpi_firmware package
    cp build/src/cpp/libfast* package
    cp build/thirdparty/fastcdr/src/cpp/libfast* package
    cp ../../cpm_lib/build_arm/libcpm.so package
    tar -czf package.tar.gz package
    popd
    
    
    # Build for normal operation on Raspberry
    pushd build_arm
    cmake .. -DBUILD_ARM=ON -DBUILD_SIMULATION=OFF -DCMAKE_TOOLCHAIN_FILE=../Toolchain.cmake
    make -j$(nproc)
    cp -R ../package/ .
    cp vehicle_rpi_firmware package
    cp build/src/cpp/libfast* package
    cp build/thirdparty/fastcdr/src/cpp/libfast* package
    cp ../../cpm_lib/build_arm/libcpm.so package
    cp ../../cpm_lib/QOS_LOCAL_COMMUNICATION.xml.template package
    tar -czf package.tar.gz package
    popd
    
    
    # Publish package via http/apache for the vehicles to download
    if [ ! -d "/var/www/html/raspberry" ]; then
        sudo mkdir -p "/var/www/html/raspberry"
        sudo chmod a+rwx "/var/www/html/raspberry"
    fi
    rm -f /var/www/html/raspberry/package.tar.gz
    #cp ./build_arm_sim/package.tar.gz /var/www/html/raspberry  # For onboard simulation
    cp ./build_arm/package.tar.gz /var/www/html/raspberry      # Normal case
    rm -f /var/www/html/raspberry/DDS_DOMAIN
    echo $DDS_DOMAIN >/var/www/html/raspberry/DDS_DOMAIN
    
    
    
    export IP_SELF=$(ip route get 8.8.8.8 | awk -F"src " 'NR==1{split($2,a," ");print a[1]}')
    
    if [ -z "${DISCOVERYSERVER_ID+xxx}" ]; then 
        export DISCOVERYSERVER_ID=44.53.00.5f.45.50.52.4f.53.49.4d.41
        echo "DISCOVERYSERVER_ID not set, using standard ${DISCOVERYSERVER_ID}"; 
    fi
    if [ -z "$DISCOVERYSERVER_ID" ] && [ "${DISCOVERYSERVER_ID+xxx}" = "xxx" ]; then echo "DISCOVERYSERVER_ID is set but empty, this can lead to unexpected bahaviour"; fi
    if [ -z "${DISCOVERYSERVER_PORT+xxx}" ]; then 
        export DISCOVERYSERVER_PORT=11811
        echo "DISCOVERYSERVER_PORT not set, using standard ${DISCOVERYSERVER_PORT}"; 
    fi
    if [ -z "$DISCOVERYSERVER_PORT" ] && [ "${DISCOVERYSERVER_ID+xxx}" = "xxx" ]; then echo "DISCOVERYSERVER_PORT is set but empty, this can lead to unexpected bahaviour"; fi


    rm -f /var/www/html/raspberry/DISCOVERY_SERVER
    echo $IP_SELF > /var/www/html/raspberry/DISCOVERY_SERVER
    echo $DISCOVERYSERVER_PORT >> /var/www/html/raspberry/DISCOVERY_SERVER
    echo $DISCOVERYSERVER_ID >> /var/www/html/raspberry/DISCOVERY_SERVER
fi
