#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

int main() {
    key_t key = ftok("shmfile", 65);

    // Create shared memory
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);


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
    printf("Enter a message: ");
    fgets(str, 1024, stdin);

    printf("Message written to shared memory.\n");
    // Detach
    shmdt(str);

    return 0;
}
