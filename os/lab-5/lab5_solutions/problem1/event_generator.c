#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "event.h"

void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int main(void) {
    ensure_shm_file_exists(SHM_FILE);

    key_t key = ftok(SHM_FILE, PROJECT_ID);
    if (key == (key_t)-1) {
        perror("ftok");
        exit(1);
    }

    int shmid = shmget(key, sizeof(SharedEventBuffer), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    SharedEventBuffer *shm = (SharedEventBuffer *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    printf("--- Event Generator ---\n");

    char buffer[256];
    int event_id;

    while (1) {
        printf("\nEnter Event ID (-1 to exit): ");
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }
        trim_newline(buffer);

        if (strcmp(buffer, "-1") == 0 || strcasecmp(buffer, "exit") == 0) {
            // Wait if there is still an unprocessed event
            while (shm->has_event == 1) {
                usleep(100000);
            }
            shm->terminated = 1;
            printf("Exiting event generator...\n");
            break;
        }

        event_id = atoi(buffer);
        if (event_id <= 0) {
            printf("Invalid Event ID. Please enter a positive number or -1 to exit.\n");
            continue;
        }

        // Wait if monitor hasn't read previous event yet
        if (shm->has_event == 1) {
            printf("Waiting for monitor to process previous event...\n");
            while (shm->has_event == 1) {
                usleep(100000);
            }
        }

        printf("Enter Event Type: ");
        if (!fgets(shm->event_type, sizeof(shm->event_type), stdin)) {
            break;
        }
        trim_newline(shm->event_type);

        printf("Enter Event Message: ");
        if (!fgets(shm->event_message, sizeof(shm->event_message), stdin)) {
            break;
        }
        trim_newline(shm->event_message);

        shm->event_id = event_id;

        // Timestamp
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        strftime(shm->timestamp, sizeof(shm->timestamp), "%Y-%m-%d %H:%M:%S", t);

        // Notify monitor
        shm->has_event = 1;
        printf("Event %d sent to shared memory.\n", event_id);
    }

    shmdt(shm);
    return 0;
}
