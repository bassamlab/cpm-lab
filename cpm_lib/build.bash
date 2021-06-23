#!/bin/bash
# exit when any command fails
set -e

# DIR holds the location of build.bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

rm -rf $DIR/cpm_library_package
mkdir $DIR/cpm_library_package

if [ ! -d "dds_idl_cpp" ]; then
    echo "Generating C++ IDL files..."
    ./ddsgen.bash
fi

mkdir -p $DIR/build

cd $DIR/build
cmake ..
make -C $DIR/build -j$(nproc) && $DIR/build/unittest
cd $DIR

# Publish cpm_library package via http/apache for the HLCs to download
if [ -z $SIMULATION ]; then
    if [ ! -d "/var/www/html/nuc" ]; then
        sudo mkdir -p "/var/www/html/nuc"
        sudo chmod a+rwx "/var/www/html/nuc"
    fi
    cp $DIR/build/libcpm.so $DIR/cpm_library_package
    tar -czf cpm_library_package.tar.gz -C $DIR/ cpm_library_package
    rm -f /var/www/html/nuc/cpm_library_package.tar.gz
    mv $DIR/cpm_library_package.tar.gz /var/www/html/nuc
fi

# Build the Matlab Bindings using MEX, if possible and with sudo privileges for update-alternatives
if sudo true; then
    cd $DIR/matlab_mex_bindings
    sudo bash build_mex.bash # checks for $SIMULATION by itself
    cd $DIR
else
    warning_msg="WARNING: Without sudo, the Mex Matlab Bindings will not be built! "
    warning_msg+="You may build them on your own (see build_mex.bash or README.md in matlab_mex_bindings to see how) without using sudo, "
    warning_msg+="which may mean that you cannot call update-alternatives to switch to a supported GCC version. "
    warning_msg+="This may not actually be problematic, as MEX will only print a warning in that case."
    echo $warning_msg
fi