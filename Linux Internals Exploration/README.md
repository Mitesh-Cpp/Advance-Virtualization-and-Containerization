For question 1
    Run "make"
    For lkm1:
        For inserting the module: "sudo insmod lkm1.ko"
        For checking the output on kernel logs: "sudo dmesg"
        For removing the module: "sudo rmmod lkm1"
    For lkm2:
        For inserting the module: "sudo insmod lkm2.ko given_pid=<some_pid>"
        For checking the output on kernel logs: "sudo dmesg"
        For removing the module: "sudo rmmod lkm2"
    For lkm3:
        For inserting the module: "sudo insmod lkm3.ko given_pid=<some_pid> given_virtual_address=<some_virtual_address>"
        For checking with the test program: "./lkm3_test" (Run this in another terminal. It will give the PID and virtual_address to check the functionality of lkm3.)
        For checking the output on kernel logs: "sudo dmesg"
        For removing the module: "sudo rmmod lkm3"
    For lkm4:
        For inserting the module: "sudo insmod lkm4.ko given_pid=<some_pid>"
        For checking with the test program: "./lkm4_test" (Run this in another terminal. The allocated memory will keep increasing, do insmod repeatedly to check how the total_physical_mapped_memory and total_virtual_memory_allocated for that process is changing)
        For checking the output on kernel logs: "sudo dmesg"
        For removing the module: "sudo rmmod lkm4"
        

For question 2-I
    Run "sudo bash spock.sh"


For question 2-II
    Run "sudo bash run_dr_doom.sh"
    [Added sleep() after ./control_station in bash script, so that control station can initiate properly before soldier process begins]


For question 3
    Run "make"
    For hello_procfs:
        For inserting the module: "sudo insmod hello_procfs.ko"
        For checking the output using proc: "cat /proc/hello_procfs"
        For removing the module: "sudo rmmod hello_procfs"
    For get_pgfaults:
        For inserting the module: "sudo insmod get_pgfaults.ko"
        For checking the output using proc: "cat /proc/get_pgfaults_procfs"
        For removing the module: "sudo rmmod get_pgfaults"