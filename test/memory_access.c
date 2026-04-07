#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

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
    long long hits, misses;

    for (int i = 0; i < 10000; i++) {
        long long int start, end, hit_time, miss_time;
        int fd = open("victim", O_RDONLY);

        char *map = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0);
        int offset = 4553; // 00000000000011c9 T victim
        char *adrs = map + offset;

        volatile char dummy = *adrs; 

        hit_time = probe(adrs);
        hits += hit_time;

        miss_time = probe(adrs);
        misses += miss_time;
    }

    printf("Hit: %llu cycles\n", hits / 10000);
    printf("Miss: %llu cycles\n", misses / 10000);

}