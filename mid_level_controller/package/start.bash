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
wget http://192.168.1.249/raspberry/DISCOVERY_SERVER

declare -a discovery_server
while read -r; do
  discovery_server+=( "$REPLY" )
done <DISCOVERY_SERVER


LD_LIBRARY_PATH=/tmp/package chrt -r 98 ./vehicle_rpi_firmware --dds_domain=$DDS_DOMAIN --simulated_time=false --discovery_mode=client --discovery_server_ip=${discovery_server[0]} --discovery_server_port=${discovery_server[1]} --discovery_server_id=${discovery_server[2]} --vehicle_id=$VEHICLE_ID --realtime=1 >stdout_$VEHICLE_ID.txt 2>stderr_$VEHICLE_ID.txt
