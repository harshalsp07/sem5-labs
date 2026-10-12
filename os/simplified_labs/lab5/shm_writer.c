/*
 * Lab 5: Shared Memory Writer (Producer)
 *
 * Purpose:
 *   Creates a shared memory segment using System V IPC (shmget),
 *   attaches it to its address space (shmat), writes a string,
 *   and detaches (shmdt).
 *
 * Compilation:
 *   gcc shm_writer.c -o shm_writer
 *
 * Usage:
 *   ./shm_writer
 *   (Then run ./shm_reader in another terminal or after this)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 5678
#define SHM_SIZE 1024

int main() {
    // 1. Create/get shared memory segment
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }

    // 2. Attach shared memory segment to process memory space
    char *shared_memory = (char *)shmat(shmid, NULL, 0);
    if (shared_memory == (char *)-1) {
        perror("shmat failed");
        return 1;
    }

    // 3. Write data into shared memory
    const char *message = "Hello! This message is stored in Shared Memory.";
    strncpy(shared_memory, message, SHM_SIZE);

    printf("=========================================\n");
    printf(" Shared Memory Writer\n");
    printf(" Written to shared memory: \"%s\"\n", shared_memory);
    printf(" Now run './shm_reader' to read it!\n");
    printf("=========================================\n");

    // 4. Detach shared memory
    shmdt(shared_memory);

    return 0;
}
