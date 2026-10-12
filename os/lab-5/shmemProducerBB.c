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

    /* Create shared memory */
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    /* Set shared memory size */
    ftruncate(shm_fd, sizeof(SharedMemory));

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

    /* Initialize buffer indexes */
    shm->in = 0;
    shm->out = 0;

    /* Produce 10 items */
    for (int item = 1; item <= 10; item++) {

        /* Wait if buffer is full */
        while ((shm->in + 1) % BUFFER_SIZE == shm->out) {
            printf("Buffer full. Producer waiting...\n");
            sleep(1);
        }

        /* Put item into buffer */
        shm->buffer[shm->in] = item;

        printf("Producer produced: %d\n", item);

        /* Move in pointer */
        shm->in = (shm->in + 1) % BUFFER_SIZE;

        sleep(1);
    }

    printf("Producer finished.\n");

    munmap(shm, sizeof(SharedMemory));
    close(shm_fd);

    return 0;
}
