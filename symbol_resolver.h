#ifndef SYMBOL_RESOLVER_H
#define SYMBOL_RESOLVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sched.h>
#include <stdbool.h>

typedef struct ProbeAddresses {
    unsigned long mul;
    unsigned long div;
    unsigned long sqr;
} ProbeAddresses;

// Automatically extract symbol addresses from binary
int resolve_symbols(const char *binary_path, ProbeAddresses *addrs) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "nm %s | grep -E 'mul_n|mpihelp_divrem|mpih_sqr_n'", binary_path);
    
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run nm command\n");
        return 0;
    }
    
    char line[256];
    int found = 0;
    
    while (fgets(line, sizeof(line), fp) != NULL) {
        unsigned long addr;
        char type;
        char name[128];
        
        if (sscanf(line, "%lx %c %s", &addr, &type, name) == 3) {
            if (strcmp(name, "mul_n") == 0) {
                addrs->mul = addr;
                found++;
                printf("Found mul_n at 0x%lx\n", addr);
            } else if (strcmp(name, "mpihelp_divrem") == 0) {
                addrs->div = addr;
                found++;
                printf("Found mpihelp_divrem at 0x%lx\n", addr);
            } else if (strcmp(name, "mpih_sqr_n") == 0) {
                addrs->sqr = addr;
                found++;
                printf("Found mpih_sqr_n at 0x%lx\n", addr);
            }
        }
    }
    
    pclose(fp);
    
    if (found != 3) {
        fprintf(stderr, "Could not find all symbols (found %d/3)\n", found);
        fprintf(stderr, "Make sure GPG was compiled with symbols or use fallback addresses\n");
        return 0;
    }
    
    return 1;
}

// Fallback to hardcoded addresses if symbol resolution fails
void use_fallback_addresses(ProbeAddresses *addrs) {
    addrs->mul = 0xa3557;
    addrs->div = 0xa1d80;
    addrs->sqr = 0xa2e77;
    printf("Using fallback addresses:\n");
    printf("  MUL: 0x%lx\n", addrs->mul);
    printf("  DIV: 0x%lx\n", addrs->div);
    printf("  SQR: 0x%lx\n", addrs->sqr);
}

#endif
