# Get and go to LCC Path
LCC_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/"
cd ${LCC_PATH}

# Load environment Variables
export IP_SELF=$(ip route get 8.8.8.8 | awk -F"src " 'NR==1{split($2,a," ");print a[1]}')

if [ -z "${DISCOVERYSERVER_ID+xxx}" ]; then 
export DISCOVERYSERVER_ID=44.53.00.5f.45.50.52.4f.53.49.4d.41
echo "DISCOVERYSERVER_ID not set at all, standard ${DISCOVERYSERVER_ID} is used"; 
fi
if [ -z "$DISCOVERYSERVER_ID" ] && [ "${DISCOVERYSERVER_ID+xxx}" = "xxx" ]; then echo "DISCOVERYSERVER_ID is set but empty, this can lead to unexpected bahaviour"; fi
if [ -z "${DISCOVERYSERVER_PORT+xxx}" ]; then 
export DISCOVERYSERVER_PORT=11811
echo "DISCOVERYSERVER_PORT not set at all, standard ${DISCOVERYSERVER_PORT} is used"; 
fi
if [ -z "$DISCOVERYSERVER_PORT" ] && [ "${DISCOVERYSERVER_ID+xxx}" = "xxx" ]; then echo "DISCOVERYSERVER_PORT is set but empty, this can lead to unexpected bahaviour"; fi

./build/lab_control_center --dds_domain=$DDS_DOMAIN --client_server=server --discovery_server_id=$DISCOVERYSERVER_ID --discovery_server_ip=$IP_SELF --discovery_server_port=$DISCOVERYSERVER_PORT --number_of_vehicles=20