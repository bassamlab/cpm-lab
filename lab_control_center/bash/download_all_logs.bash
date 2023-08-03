#!/bin/bash

# Tries to download all logs from available NUCs and PIs

IDS=$'
01
02
03
04
05
06
07
08
09
10
11
12
13
14
15
16
17
18
19'

folder=$1

# On the pi
# /tmp/package/stderr_11.txt
# /tmp/package/stdout_11.txt
# /tmp/package/Log_2023_03_10_16_11_07.csv
# On the NUC when an experiment took place
# dev/lcc_script_logs/
# middleware.log  script.log  script_path.log  tmux_middleware.txt  tmux_script.txt
for id in $IDS;
do
    echo "Try to retrieve log from NUC and vehicle $id"
    sshpass -p "cpmcpmcpm" scp pi@192.168.1.1$id:/tmp/package/stderr_* $folder/stderr_pi_$id.log &
    sshpass -p "cpmcpmcpm" scp pi@192.168.1.1$id:/tmp/package/stdout_* $folder/stdout_pi_$id.log &
    sshpass -p "cpmcpmcpm" scp pi@192.168.1.1$id:/tmp/package/Log* $folder/log_pi_$id.log &
    scp guest@192.168.1.2$id:/home/guest/dev/lcc_script_logs/middleware.log $folder/middleware_nuc_$id.log &
    scp guest@192.168.1.2$id:/home/guest/dev/lcc_script_logs/script.log $folder/hlc_nuc_$id.log &
done

# Run last command in foreground so that hopefully all output is given until the shell allows the next command
echo "Retrieve log from vehicle 20"
sshpass -p "cpmcpmcpm" scp pi@192.168.1.120:/tmp/package/Log* $folder/id_$id/log_veh_$id.log &
echo "Retrieve log from NUC 20"
scp guest@192.168.1.2$id:/home/guest/dev/lcc_script_logs/*  $folder/id_$id/ &
