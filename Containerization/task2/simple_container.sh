#!/bin/bash

SIMPLE_CONTAINER_ROOT=container_root

mkdir -p $SIMPLE_CONTAINER_ROOT

gcc -o container_prog container_prog.c

## Subtask 1: Execute in a new root filesystem

cp container_prog $SIMPLE_CONTAINER_ROOT/

# 1.1: Copy any required libraries to execute container_prog to the new root container filesystem 
list="$(ldd ./container_prog | egrep -o '/lib.*\.[0-9]')"
for i in $list; do cp -v --parents "$i" "${SIMPLE_CONTAINER_ROOT}"; done
# got the above syntax from the reference provided in the assignment document: "https://www.howtogeek.com/441534/how-to-use-the-chroot-command-on-linux/"

echo -e "\n\e[1;32mOutput Subtask 2a\e[0m"
# 1.2: Execute container_prog in the new root filesystem using chroot. You should pass "subtask1" as an argument to container_prog
sudo chroot $SIMPLE_CONTAINER_ROOT /container_prog subtask1


echo "__________________________________________"
echo -e "\n\e[1;32mOutput Subtask 2b\e[0m"
## Subtask 2: Execute in a new root filesystem with new PID and UTS namespace
# The pid of container_prog process should be 1
# You should pass "subtask2" as an argument to container_prog
sudo unshare --fork --pid --uts chroot $SIMPLE_CONTAINER_ROOT /container_prog subtask2
# got to know of the above syntax from the examples provided in "https://man7.org/linux/man-pages/man1/unshare.1.html"


echo -e "\nHostname in the host: $(hostname)"


## Subtask 3: Execute in a new root filesystem with new PID, UTS and IPC namespace + Resource Control
# Create a new cgroup and set the max CPU utilization to 50% of the host CPU. (Consider only 1 CPU core)
sudo mkdir -p /sys/fs/cgroup/cpu_50_percent/
sudo echo "50000 100000" > /sys/fs/cgroup/cpu_50_percent/cpu.max
# got to know of above syntax and methodology from: "https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/managing_monitoring_and_updating_the_kernel/using-cgroups-v2-to-control-distribution-of-cpu-time-for-applications_managing-monitoring-and-updating-the-kernel#proc_controlling-distribution-of-cpu-time-for-applications-by-adjusting-cpu-bandwidth_using-cgroups-v2-to-control-distribution-of-cpu-time-for-applications" Section: "24.3. Controlling distribution of CPU time for applications by adjusting CPU bandwidth"

echo "__________________________________________"
echo -e "\n\e[1;32mOutput Subtask 2c\e[0m"
# Assign pid to the cgroup such that the container_prog runs in the cgroup
sudo echo $$ > /sys/fs/cgroup/cpu_50_percent/cgroup.procs

# Run the container_prog in the new root filesystem with new PID, UTS and IPC namespace
sudo unshare --fork --pid --uts --ipc chroot $SIMPLE_CONTAINER_ROOT /container_prog subtask3

# You should pass "subtask1" as an argument to container_prog

# Remove the cgroup
# tried removing the directory direcly, but was unable to do so
# thats why first moved the pid to the root's cgroup and then removed the directory
# this worked..:-)
echo $$ > /sys/fs/cgroup/cgroup.procs
rmdir /sys/fs/cgroup/cpu_50_percent/

# If mounted dependent libraries, unmount them, else ignore