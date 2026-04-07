#ifndef MY_SPY_H
#define MY_SPY_H

// 000000000009f390 T mpih_sqr_n_basecase
// 000000000009fa00 T mpihelp_mul
// 000000000009e2e0 T mpihelp_divrem
// 000000000009fbc0 T mpihelp_mul_karatsuba_case

#define THRESHOLD 80
#define MAX_PROBES 20000000
#define BUSY 2500

#define SQR 0x9f390 
#define MUL 0x9fbc0
#define DIV 0x9e2e0

#endif