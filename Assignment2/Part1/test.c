#include <stdlib.h>
#include <stdio.h>

int main() {
    int mem_request = 66;
    int actual_allocated_mem = 1;
    int level = 0;

    while (!(mem_request / actual_allocated_mem >= 1 && mem_request / actual_allocated_mem < 2)) {
        level++;
        actual_allocated_mem = 1 << level;
    }

    printf("mem: %d, level: %d\n", actual_allocated_mem, level);

    return 0;

}
