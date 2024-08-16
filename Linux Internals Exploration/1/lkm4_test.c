#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main() {
    int *check_array;
    printf("Process id = %d\n", getpid());

    for (int i = 1; i <= 100; i+=20) {
        int size = 1024 * i * 10;

        check_array = (int *)malloc(size * sizeof(int));
        if (check_array == NULL) {
            printf("Error while allocating array..!!\n");
            return 0;
        }
        printf("Memory allocated: %d KB\n", 4 * i * 10);
        printf("Press any key to continue\n");
        getchar();
        free(check_array);
    }
    return 0;
}