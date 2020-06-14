#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

/**
 * Sample process that adds 1 to 1000000000 * count.
 * 
 * Takes about (2 * count)s in Intel(R) Core(TM) i9-8950HK CPU @ 2.90GHz
 **/
volatile int main(int argc, char *argv[]) {
    if (argc < 1) {
        fprintf(stderr, "Usage: %s [Count]\n", argv[0]);
        exit(-1);
    }
    int count = atoi(argv[1]);
    uint64_t sum = 0;
    for (uint64_t i = 0; i < (uint64_t) 10000000LL * count; i++) {
        sum += i;
    }
    printf("[*] process1 end\n");
    return 0;
}