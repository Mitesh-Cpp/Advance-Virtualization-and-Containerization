#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>

// Include header or define the IOCTL call interface and devide name
#include "my_ioctl_header.h"

//**************************************************

int open_driver(const char* driver_name) {

    int fd_driver = open(driver_name, O_RDWR);
    if (fd_driver == -1) {
        perror("ERROR: could not open driver");
    }

	return fd_driver;
}

void close_driver(const char* driver_name, int fd_driver) {

    int result = close(fd_driver);
    if (result == -1) {
        perror("ERROR: could not close driver");
    }
}


int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: %s <parent_pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t parent_pid = atoi(argv[1]);


    // open ioctl driver
    int device_fd = open_driver("/dev/my_char_device");
    if(!device_fd) 
        printf("Error opening file..!!\n");
    // call ioctl with parent pid as argument to change the parent
    struct SIGCHLD_SIGNAL ss = {
        .new_parent_process_id = parent_pid
    };
    ioctl(device_fd, IOCTL_SEND_SIGCHLD_SIGNAL, &ss);
    // close ioctl driver
    close_driver("/dev/my_doom_device", device_fd);
	return EXIT_SUCCESS;
}