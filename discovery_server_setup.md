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
