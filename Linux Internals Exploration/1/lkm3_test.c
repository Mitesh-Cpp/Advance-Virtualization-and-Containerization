#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

// dummy program to allocate some memory on heap and get its virtual address for checking the working of lkm3

int main()
{
    printf("PID: %d\n", getpid());
    int *ptr = (int *)malloc(5000);
    printf("VIRTUAL ADDRESS: %lu\n", ptr);
    printf("Enter any character to terminate the program..!!\n");
    getchar();
}