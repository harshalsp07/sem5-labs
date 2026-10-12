/*
 * Lab 5: Producer-Consumer Problem (Bounded Circular Buffer)
 *
 * Purpose:
 *   Demonstrates the classic Bounded Buffer Producer-Consumer problem.
 *   - A shared circular buffer of fixed size (BUFFER_SIZE = 5).
 *   - The Producer inserts items into buffer[in].
 *   - The Consumer removes items from buffer[out].
 *   - Uses fork() with shared memory so parent and child run concurrently.
 *
 * Compilation:
 *   gcc producer_consumer.c -o producer_consumer
 *
 * Usage:
 *   ./producer_consumer
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define BUFFER_SIZE 5
#define TOTAL_ITEMS 10

typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    int count; // Number of items currently in buffer
} SharedData;

int main() {
    // 1. Create shared memory
    int shmid = shmget(IPC_PRIVATE, sizeof(SharedData), 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }

    SharedData *shared = (SharedData *)shmat(shmid, NULL, 0);
    shared->in = 0;
    shared->out = 0;
    shared->count = 0;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // --- CONSUMER (Child Process) ---
        for (int i = 1; i <= TOTAL_ITEMS; i++) {
            // Busy-wait while buffer is empty
            while (shared->count == 0) {
                usleep(10000); // Sleep 10ms to prevent CPU burn
            }

            int item = shared->buffer[shared->out];
            printf("  [CONSUMER] Consumed item: %d from slot %d (count: %d)\n",
                   item, shared->out, shared->count - 1);

            shared->out = (shared->out + 1) % BUFFER_SIZE;
            shared->count--;

            usleep(150000); // Simulate processing time
        }

        shmdt(shared);
        exit(0);
    } else {
        // --- PRODUCER (Parent Process) ---
        for (int item = 1; item <= TOTAL_ITEMS; item++) {
            // Busy-wait while buffer is full
            while (shared->count == BUFFER_SIZE) {
                usleep(10000); // Sleep 10ms
            }

            shared->buffer[shared->in] = item;
            printf("[PRODUCER] Produced item: %d at slot %d (count: %d)\n",
                   item, shared->in, shared->count + 1);

            shared->in = (shared->in + 1) % BUFFER_SIZE;
            shared->count++;

            usleep(100000); // Simulate production time
        }

        // Wait for consumer to finish consuming all items
        wait(NULL);

        // Detach and clean up shared memory
        shmdt(shared);
        shmctl(shmid, IPC_RMID, NULL);

        printf("All items produced and consumed successfully.\n");
    }

    return 0;
}
