/*
 * Lab 1: Disk I/O Demo
 *
 * Purpose:
 *   Demonstrates file write and read operations to generate disk I/O activity.
 *   This can be monitored in real-time with tools like: iotop or iostat.
 *
 * Compilation:
 *   gcc disk_demo.c -o disk_demo
 *
 * Usage:
 *   ./disk_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FILENAME "temp_disk_test.dat"
#define BLOCK_SIZE 1024       // 1 KB block
#define NUM_BLOCKS 10240      // 10 MB total

int main() {
    char buffer[BLOCK_SIZE];
    memset(buffer, 'X', sizeof(buffer));

    printf("=========================================\n");
    printf(" Disk I/O Demo\n");
    printf(" Process PID: %d\n", getpid());
    printf(" Writing %d MB to '%s'...\n", (NUM_BLOCKS * BLOCK_SIZE) / (1024 * 1024), FILENAME);

    // 1. Write blocks to disk
    FILE *fp = fopen(FILENAME, "wb");
    if (!fp) {
        perror("Failed to open file for writing");
        return 1;
    }

    for (int i = 0; i < NUM_BLOCKS; i++) {
        fwrite(buffer, 1, BLOCK_SIZE, fp);
    }
    fclose(fp);
    printf(" Finished writing to disk.\n");

    // 2. Read blocks back from disk
    printf(" Reading %d MB back from '%s'...\n", (NUM_BLOCKS * BLOCK_SIZE) / (1024 * 1024), FILENAME);
    fp = fopen(FILENAME, "rb");
    if (!fp) {
        perror("Failed to open file for reading");
        return 1;
    }

    size_t total_read = 0;
    while (fread(buffer, 1, BLOCK_SIZE, fp) > 0) {
        total_read += BLOCK_SIZE;
    }
    fclose(fp);

    printf(" Read complete (%zu bytes).\n", total_read);

    // 3. Clean up the temporary file
    remove(FILENAME);
    printf(" Temporary file deleted successfully.\n");
    printf("=========================================\n");

    return 0;
}
