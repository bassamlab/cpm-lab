# Get and go to LCC Path
LCC_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/"
cd ${LCC_PATH}

# Load environment Variables
export IP_SELF=$(ip route get 8.8.8.8 | awk -F"src " 'NR==1{split($2,a," ");print a[1]}')
export DDS_INITIAL_PEER=rtps@udpv4://$IP_SELF:25598

./build/lab_control_center --dds_domain=$DDS_DOMAIN --client_server=server --discovery_server_id=44.53.00.5f.45.50.52.4f.53.49.4d.41 --discovery_server_ip=192.168.1.249 --discovery_server_port=11811 --number_of_vehicles=20