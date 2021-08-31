TODO later on: Automate installation

# Setting up the Discovery Server
For better performance, one or multiple discovery servers must be set up in the lab. This manual tells you how to install it:
https://eprosima-discovery-server.readthedocs.io/en/latest/installation/installation_linux.html

## Preparation
The following things need to be considered:
- You must have installed eProsima already, which is done by the install script in this repository. 
- Then, the mentioned python packages need to be installed as a preparation:
```bash
sudo apt install cmake g++ python3-pip wget git libasio-dev libtinyxml2-dev libssl-dev
pip3 install jsondiff==1.2.0 xmltodict==0.12.0
pip3 install -U colcon-common-extensions vcstool
```

## Installation
```bash
mkdir -p discovery-server-ws/src && cd discovery-server-ws
wget https://raw.githubusercontent.com/eProsima/Discovery-Server/master/discovery-server.repos
vcs import src < discovery-server.repos

colcon build --base-paths src \
        --packages-up-to discovery-server \
        --cmake-args -DLOG_LEVEL_INFO=ON -DCOMPILE_EXAMPLES=ON -DINTERNALDEBUG=ON -DCMAKE_BUILD_TYPE=Debug
```

This installation is not what we want to use though. It ignores that all relevant prerequisites are already installed (you should have FastDDS installed globally already, as well as all other dependencies, using the install script). Thus, it is better to do the following:

```bash
git clone https://github.com/eProsima/Discovery-Server.git
cd Discovery-Server
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -C ../build -j$(nproc)
cd ..
```

This can also be done for the examples, if desired.

# Using the Discovery Server
## Setup in our current framework
From the eProsima docs: "Clients require a beforehand knowledge of the servers to which they want to link. Basically it is reduced to the servers identity (henceforth called GuidPrefix_t) and a list of locators where the servers are listening. These locators also define the transport protocol (UDP or TCP) the client will use to contact the server."

The prefix is used as it is more specific than an IP - multiple servers may be running on one machine. 

Setup: Change the participant QoS
```C++
DomainParticipantQos clientQos = PARTICIPANT_QOS_DEFAULT;

clientQos.wire_protocol().builtin.discovery_config.discoveryProtocol =
        DiscoveryProtocol_t::CLIENT;

// Setting the server ID
RemoteServerAttributes server_attributes;
server_attributes.ReadguidPrefix("44.53.00.5f.45.50.52.4f.53.49.4d.41");

// Setting the server locator list
Locator_t locator;
IPLocator::setIPv4(locator, 192, 168, 1, 133); //SERVER IP!
locator.port = 64863; //SERVER PORT!
server_attributes.metatrafficUnicastLocatorList.push_back(locator); //Multicast or Unicast
clientQos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(server_attributes);

// Specify the sync. period w.r.t. discovery messages to the server
participant_qos.wire_protocol().builtin.discovery_config.discoveryServer_client_syncperiod =
        Duration_t(0, 250000000);
```

A client then only receives the relevant information for matching etc. from the server.

## Setup of the Server Itself
```C++
DomainParticipantQos serverQos = PARTICIPANT_QOS_DEFAULT;

serverQos.wire_protocol().builtin.discovery_config.discoveryProtocol =
        DiscoveryProtocol_t::SERVER;

// Setting up the ID of the server
std::istringstream("44.53.00.5f.45.50.52.4f.53.49.4d.41") >> serverQos.wire_protocol().prefix;   

// Setting the locator list (here: PDP)
Locator_t locator;
IPLocator::setIPv4(locator, 192, 168, 1, 133); //IP!
locator.port = 64863; //PORT!
serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);
```