/*
 * Lab 1: CPU Utilization Demo
 * 
 * Purpose:
 *   Runs a busy computational loop to utilize CPU cycles.
 *   This allows you to observe CPU usage in another terminal
 *   using tools like: top, htop, or ps.
 *
 * Compilation:
 *   gcc cpu_demo.c -o cpu_demo
 *
 * Usage:
 *   ./cpu_demo
 */

#include <stdio.h>
#include <unistd.h>

int main() {
    printf("=========================================\n");
    printf(" CPU Utilization Demo\n");
    printf(" Process PID: %d\n", getpid());
    printf(" Open another terminal and run: top -p %d\n", getpid());
    printf(" Press Ctrl+C to stop.\n");
    printf("=========================================\n");

    // Infinite busy loop doing simple arithmetic
    volatile unsigned long counter = 0;
    while (1) {
        counter++;
        if (counter % 500000000 == 0) {
            printf("[PID %d] Still running... counter = %lu\n", getpid(), counter);
        }
    }

    return 0;
}
