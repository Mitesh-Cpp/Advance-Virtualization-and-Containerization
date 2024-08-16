#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "my_ioctl_header.h"

int main() {
    printf("\n---------------------\n");
    unsigned char *allocated_byte_address = malloc(sizeof(unsigned char));
    *allocated_byte_address = 6;
    printf("Virtual address: %p\nValue: %d\n\n", allocated_byte_address, *allocated_byte_address);

    // Opening the device file
    int fd = open("/dev/my_char_device", O_RDWR);
    if (fd < 0) {
        printf("File not found..!!\n");
        perror("open");
        exit(1);
    }

    // Getting the physical address using ioctl call
    unsigned long physical_addr;
    struct GET_P_ADDR gpa = {
        .virtual_addr = (unsigned long)allocated_byte_address
    };
    ioctl(fd, IOCTL_GET_PHYSICAL_ADDR, &gpa);
    printf("Physical address: 0x%lx\n", gpa.physical_addr);

    // Changing the value using ioctl call
    struct WRITE_P_MEM wpm = {
        .physical_addr = gpa.physical_addr,
        .value = 5
    };
    ioctl(fd, IOCTL_WRITE_PHYSICAL_MEM, &wpm);
    printf("Modified value: %d\n", *allocated_byte_address);
    close(fd);
    return 0;
}