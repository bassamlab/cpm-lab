#!/bin/bash

if [ ! -d "./build" ]
then
    printf "Please run ./build.bash first.\n"
    exit 1
fi

if [ $# -eq 0 ]
then
    printf "Usage (Example):\nCreate Vehicle in LCC first, then:\n./run_distributed.bash --vehicle_ids=1,2,3,4\n"
    exit 1
fi

for arg in "$@"
do
    case $arg in
        -id*|--vehicle_ids=*)
            vehicle_ids="${arg#*=}"
            vehicle_array=(${vehicle_ids//,/ })
            shift
            ;;
        --debug)
            debug=true
            shift
            ;;
        --show_logs)
            show_logs=true
            shift
            ;;
        *)
            ;;
    esac
done

printf "vehicle_ids: ${vehicle_ids}\n"
printf "DDS_DOMAIN: ${DDS_DOMAIN}\n"

if [ ${#vehicle_array[@]} -gt 5 ]
then
    printf "Warning: Using more than 5 vehicles during local testing \
can lead to unexpected results\n"
    # Reason: RTI DDS Participants only search for 4 other Participants
    # per domain on a single machine per default.
fi

# Get environment variables directory relative to this script's directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/"
RELATIVE_BASH_FOLDER_DIR="${DIR}../../../../lab_control_center/bash"
ABSOLUTE_BASH_FOLDER_DIR="$(realpath "${RELATIVE_BASH_FOLDER_DIR}")"
RELATIVE_SOFTWARE_FOLDER_DIR="${DIR}../../../../"
ABSOLUTE_SOFTWARE_FOLDER_DIR="$(realpath "${RELATIVE_SOFTWARE_FOLDER_DIR}")"
RELATIVE_LOGS_FOLDER_DIR="${DIR}../../../../../lcc_script_logs"
ABSOLUTE_LOGS_FOLDER_DIR="$(realpath "${RELATIVE_LOGS_FOLDER_DIR}")"

. ${ABSOLUTE_BASH_FOLDER_DIR}/environment_variables_local.bash

cleanup(){
    printf "Cleaning up ... "

    for vehicle_id in "${vehicle_array[@]}"
    do
        tmux kill-session -t distributed_routing_${vehicle_id}
        tmux kill-session -t middleware_${vehicle_id}
    done

    # Kill all children
    kill 0
    printf "Done.\n"
}
trap cleanup EXIT

sleep 3
cd build
for vehicle_id in "${vehicle_array[@]}"
do


    # Special debug option: if debug is enabled, the 5th vehicle will be started directly in gdb
    # This is useful, when the HLC is crashing before we have time to attach a debugger
    if [  $debug ]; then
        # This starts every HLC with a debugger attached and in separate tmux sessions
        # Use e.g. 'tmux attach-session -t distributed_routing_2' to start debugging
        # HLC 2
        tmux new-session -d -s "distributed_routing_${vehicle_id}" ". ~/dev/software/lab_control_center/bash/environment_variables_local.bash;cd /home/dev/dev/software/high_level_controller/examples/cpp/distributed_routing/build/;\
            gdb -ex=r --args ./distributed_routing \
            --node_id=high_level_controller${vehicle_id} \
            --simulated_time=false \
            --vehicle_ids=${vehicle_id} \
            --middleware=true \
	    --middleware_domain=${vehicle_id} \
            --dds_domain=${DDS_DOMAIN} \
            --dds_initial_peer=${DDS_INITIAL_PEER}"

        # Start middleware
        middleware_cmd="gdb -ex=r --args ./middleware \
            --node_id=middleware_${vehicle_id} \
            --simulated_time=false \
            --vehicle_ids=${vehicle_id} \
            --domain_number=${vehicle_id} \
            --dds_domain=${DDS_DOMAIN} \
            --dds_initial_peer=${DDS_INITIAL_PEER}"
            #>~/dev/lcc_script_logs/stdout_middleware_${vehicle_id}.txt \
            #2>~/dev/lcc_script_logs/stderr_middleware_${vehicle_id}.txt"
        tmux new-session -d -s "middleware_${vehicle_id}" ". ~/dev/software/lab_control_center/bash/environment_variables_local.bash;cd ~/dev/software/middleware/build/;${middleware_cmd}"
    printf "\tStarting middleware_${vehicle_id} ...\n"
    else
        # This starts the high level controller
        ./distributed_routing \
            --node_id=high_level_controller_${vehicle_id} \
            --simulated_time=false \
            --vehicle_ids=${vehicle_id} \
            --middleware=true \
	    --middleware_domain=${vehicle_id} \
            --dds_domain=${DDS_DOMAIN} \
            --dds_initial_peer=${DDS_INITIAL_PEER}  \
            >${ABSOLUTE_LOGS_FOLDER_DIR}/stdout_hlc${vehicle_id}.txt \
            2>${ABSOLUTE_LOGS_FOLDER_DIR}/stderr_hlc${vehicle_id}.txt&
	
        # Start middleware
    middleware_cmd="./middleware \
        --node_id=middleware_${vehicle_id} \
        --simulated_time=false \
        --vehicle_ids=${vehicle_id} \
        --domain_number=${vehicle_id} \
        --dds_domain=${DDS_DOMAIN} \
        --dds_initial_peer=${DDS_INITIAL_PEER}  \
        >${ABSOLUTE_LOGS_FOLDER_DIR}/stdout_middleware_${vehicle_ids}.txt \
        2>${ABSOLUTE_LOGS_FOLDER_DIR}/stderr_middleware_${vehicle_ids}.txt"
    tmux new-session -d -s "middleware_${vehicle_id}" ". ${ABSOLUTE_BASH_FOLDER_DIR}/environment_variables_local.bash;cd ${ABSOLUTE_SOFTWARE_FOLDER_DIR}/middleware/build/;${middleware_cmd}"
    printf "\tStarting middleware_${vehicle_id} ...\n"

    fi
    printf "\tStarting high_level_controller_${vehicle_id}.\n"
done
printf "Done.\n\n"

printf "To abort, press Ctrl+C\n"

# This displays all log files of hlcs in lcc_script_logs
# This may be more than we actually created
if [ $show_logs ]; then
    printf "Displaying stdout and stderr of all started High Level Controller\n"
    tail -f ${ABSOLUTE_LOGS_FOLDER_DIR}/*.txt
fi

while true
do
    sleep 1
done
