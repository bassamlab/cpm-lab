#!/bin/bash

sudo arp -f ~/arp_table
export IP_SELF="$(hostname -I)"

echo IP_SELF
echo $IP_SELF

# Copy local communication QoS, use correct IP
sed -e "s/TEMPLATE_IP/${IP_SELF}/g" \
<$DIR/QOS_LOCAL_COMMUNICATION.xml.template \
>$DIR/build/QOS_LOCAL_COMMUNICATION.xml

export VEHICLE_ID="$(hostname -I | tail -c 4)"
export VEHICLE_ID="$(echo $VEHICLE_ID)"


echo VEHICLE_ID
echo $VEHICLE_ID

wget http://192.168.1.249/raspberry/DDS_DOMAIN
wget http://192.168.1.249/raspberry/DDS_INITIAL_PEER


LD_LIBRARY_PATH=/tmp/package chrt -r 98 ./vehicle_rpi_firmware --dds_domain=$DDS_DOMAIN --simulated_time=false --client_server=client --discovery_server_id=44.53.00.5f.45.50.52.4f.53.49.4d.41 --discovery_server_ip=192.168.1.249 --discovery_server_port=11811 --vehicle_id=$VEHICLE_ID --realtime=1 >stdout_$VEHICLE_ID.txt 2>stderr_$VEHICLE_ID.txt
