#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include "event.h"

static int g_shmid = -1;
static SharedEventBuffer *g_shm = NULL;

void cleanup_and_exit(int sig) {
    (void)sig;
    if (g_shm != NULL && g_shm != (void *)-1) {
        shmdt(g_shm);
    }
    if (g_shmid != -1) {
        shmctl(g_shmid, IPC_RMID, NULL);
    }
    printf("\nShared memory cleaned up. Exiting.\n");
    exit(0);
}

int main(void) {
    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);

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
    g_shmid = shmid;

    SharedEventBuffer *shm = (SharedEventBuffer *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("shmat");
        exit(1);
    }
    g_shm = shm;

    shm->has_event = 0;
    shm->terminated = 0;

    printf("--- Event Monitor ---\n");
    printf("Waiting for events from generator...\n\n");

    while (1) {
        if (shm->has_event == 1) {
            printf("New Event Received:\n");
            printf("  Event ID      : %d\n", shm->event_id);
            printf("  Event Type    : %s\n", shm->event_type);
            printf("  Event Message : %s\n", shm->event_message);
            printf("  Timestamp     : %s\n\n", shm->timestamp);

            // Mark as processed
            shm->has_event = 0;
        }

        // Terminate when generator is done and no events left
        if (shm->terminated == 1 && shm->has_event == 0) {
            printf("Generator terminated. All events processed.\n");
            break;
        }

        usleep(100000);
    }

    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    printf("Shared memory removed. Monitor exiting.\n");
    return 0;
}
