#!/bin/bash
# exit when any command fails
set -e

export RASPBIAN_TOOLCHAIN=/opt/cross-pi-gcc

if [ ! -d "build_arm" ]; then
    mkdir -p build_arm
fi

if [ ! -d "dds_idl_cpp" ]; then
    ./ddsgen.bash
fi

cd thirdparty/foonathan_memory_vendor/
mkdir -p build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../../../Toolchain.cmake -DFOONATHAN_MEMORY_BUILD_TOOLS=OFF -DFOONATHAN_MEMORY_BUILD_TESTS=OFF ..

cd ../../../build_arm
cmake .. -DCMAKE_TOOLCHAIN_FILE=../Toolchain.cmake -DBUILD_ARM=ON
make -j$(nproc)
cd ..
