#!/bin/bash
make -s
# inserting the module
sudo insmod driver.ko
# enhancing permissions of device file in case not called with "sudo"
sudo chmod 777 /dev/my_char_device
# running the user program
gcc -w -o user_program user_program.c
./user_program
# cleanup
sudo rmmod driver
make -s clean
sudo rm user_program