# Get path to current folder
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Adapt to path to libcpm s.t. Matlab can find it when calling the MEX bindings
LIBCPM_PATH=${DIR%/*}
LIBCPM_PATH=${LIBCPM_PATH%/*}
LIBCPM_PATH="$LIBCPM_PATH/cpm_lib/build/libcpm.so"

# C++ and eProsima are expected to be installed globally
export LD_PRELOAD="/usr/lib/x86_64-linux-gnu/libstdc++.so.6:${LIBCPM_PATH}:/usr/local/lib/libfastcdr.so::/usr/local/lib/libfastrtps.so"