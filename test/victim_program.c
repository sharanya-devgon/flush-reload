#include <stdio.h>
#include <unistd.h>


void victim() {
    
    fflush(stdout);
}

int main() {
    printf("PID: %d\n", getpid());
    while (1) {
        victim();
        sleep(2);
    }
    return 0;
}