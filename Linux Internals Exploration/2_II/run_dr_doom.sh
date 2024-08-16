#!/bin/bash

## Building the applications
make -s
make -f Makefile_c_files

## Build your ioctl driver and load it here
sudo insmod doom.ko
sudo chmod 777 /dev/my_char_device

###############################################

# Launching the control station
./control_station &
c_pid=$!
echo "--------------------"
echo "Control station PID: $c_pid"
sleep 4
# Launching the soldier
./soldier $c_pid &
echo "Soldier PID: $!"
sleep 2
echo "--------------------"
kill -9 $c_pid
# Remove the driver here
sudo rmmod doom
make -s clean
make -s -f Makefile_c_files clean