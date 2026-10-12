#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    key_t key = ftok("shmfile", 65);

    // Get the existing shared memory
    int shmid = shmget(key, 1024, 0666);

    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // Attach shared memory
    char *str = (char *)shmat(shmid, NULL, 0);

    if (str == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    printf("Message read from shared memory:\n");
    printf("%s\n", str);

    // Detach
    shmdt(str);

    // Remove shared memory
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
