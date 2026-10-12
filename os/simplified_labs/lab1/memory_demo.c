/*
 * Lab 1: Memory Allocation and Usage Demo
 *
 * Purpose:
 *   Demonstrates dynamic memory allocation and writing to pages.
 *   Writing to allocated pages increases Resident Set Size (RSS),
 *   which can be observed using: top, ps aux, or free -m.
 *
 * Compilation:
 *   gcc memory_demo.c -o memory_demo
 *
 * Usage:
 *   ./memory_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE_IN_MB 50

int main() {
    size_t bytes = (size_t)SIZE_IN_MB * 1024 * 1024;

    printf("=========================================\n");
    printf(" Memory Utilization Demo\n");
    printf(" Process PID: %d\n", getpid());
    printf(" Step 1: Allocating %d MB with malloc()...\n", SIZE_IN_MB);

    char *buffer = (char *)malloc(bytes);
    if (buffer == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    printf(" Memory allocated! Check RSS in 'top -p %d'.\n", getpid());
    printf(" Press Enter to touch (write to) all memory pages...\n");
    getchar();

    printf(" Step 2: Writing data to allocated memory...\n");
    for (size_t i = 0; i < bytes; i++) {
        buffer[i] = 'A';
    }

    printf(" All memory written! RSS is now high in 'top -p %d'.\n", getpid());
    printf(" Press Enter to free memory and exit...\n");
    getchar();

    free(buffer);
    printf(" Memory freed. Exiting.\n");
    return 0;
}
