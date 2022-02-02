# This file is supposed to be used to create a package that is downloaded by the NUC AFTER IT HAS BOOTED
# -> Only files that are probably relevant to all Matlab scripts are uploaded at this points
# Scripts are still supposed to be uploaded using the LCC's UI (or your own upload script)

# exit when any command fails
set -e

# Get directory of bash script
BASH_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Publish NUC package via http/apache for the HLCs to download


cd /${BASH_DIR}
rm -rf matlab_package
mkdir matlab_package
cp ${BASH_DIR}/../examples/matlab/read_system_trigger.m ./matlab_package
cp ${BASH_DIR}/../examples/matlab/init_script.m ./matlab_package
cp ${BASH_DIR}/../examples/matlab/QOS_READY_TRIGGER.xml ./matlab_package
tar -czf matlab_package.tar.gz matlab_package
rm -f /var/www/html/nuc/matlab_package.tar.gz
cp ./matlab_package.tar.gz /var/www/html/nuc
