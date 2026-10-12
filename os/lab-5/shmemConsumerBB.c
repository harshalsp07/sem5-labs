#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define SHM_NAME "/my_shared_buffer"

typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
} SharedMemory;

int main()
{
    int shm_fd;
    SharedMemory *shm;

    /* Open existing shared memory */
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);

    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    /* Attach shared memory */
    shm = mmap(NULL,
               sizeof(SharedMemory),
               PROT_READ | PROT_WRITE,
               MAP_SHARED,
               shm_fd,
               0);

    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    /* Consume 10 items */
    for (int i = 0; i < 10; i++) {

        /* Wait if buffer is empty */
        while (shm->in == shm->out) {
            printf("Buffer empty. Consumer waiting...\n");
            sleep(1);
        }

        /* Get item from buffer */
        int item = shm->buffer[shm->out];

        printf("Consumer consumed: %d\n", item);

        /* Move out pointer */
        shm->out = (shm->out + 1) % BUFFER_SIZE;

        sleep(2);
    }

    printf("Consumer finished.\n");

    munmap(shm, sizeof(SharedMemory));
    close(shm_fd);

    /* Remove shared memory */
    shm_unlink(SHM_NAME);

    return 0;
}
