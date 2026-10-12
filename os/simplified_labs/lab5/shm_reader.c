/*
 * Lab 5: Shared Memory Reader (Consumer)
 *
 * Purpose:
 *   Accesses the shared memory segment created by shm_writer,
 *   reads and prints the message, detaches it, and removes the segment (shmctl).
 *
 * Compilation:
 *   gcc shm_reader.c -o shm_reader
 *
 * Usage:
 *   ./shm_reader
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 5678
#define SHM_SIZE 1024

int main() {
    // 1. Locate existing shared memory segment
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid < 0) {
        perror("shmget failed (Make sure you ran ./shm_writer first!)");
        return 1;
    }

    // 2. Attach shared memory
    char *shared_memory = (char *)shmat(shmid, NULL, 0);
    if (shared_memory == (char *)-1) {
        perror("shmat failed");
        return 1;
    }

    // 3. Read message from shared memory
    printf("=========================================\n");
    printf(" Shared Memory Reader\n");
    printf(" Read from shared memory: \"%s\"\n", shared_memory);
    printf("=========================================\n");

    // 4. Detach shared memory
    shmdt(shared_memory);

    // 5. Deallocate / remove shared memory segment from OS
    shmctl(shmid, IPC_RMID, NULL);
    printf("Shared memory segment cleaned up successfully.\n");

    return 0;
}
