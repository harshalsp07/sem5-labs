/*
 * Lab 5 Solutions - Problem 1: Event Monitor (Shared Memory)
 *
 * Purpose:
 *   Reads and displays events posted into shared memory by event_generator.
 *   Removes the shared memory segment when generator exits.
 *
 * Compilation:
 *   gcc event_monitor.c -o event_monitor
 *
 * Usage:
 *   ./event_monitor
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#define SHM_KEY 1111

typedef struct {
    int id;
    char type[32];
    char message[128];
    int has_event; // 1 = new event available, 0 = processed
    int finished;  // 1 = generator has closed
} EventBuffer;

static int g_shmid = -1;
static EventBuffer *g_event = NULL;

void handle_exit(int sig) {
    (void)sig;
    if (g_event != NULL && g_event != (EventBuffer *)-1) shmdt(g_event);
    if (g_shmid != -1) shmctl(g_shmid, IPC_RMID, NULL);
    printf("\nShared memory cleared. Monitor exited.\n");
    exit(0);
}

int main() {
    signal(SIGINT, handle_exit);

    // 1. Create / connect to shared memory
    int shmid = shmget(SHM_KEY, sizeof(EventBuffer), 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }
    g_shmid = shmid;

    EventBuffer *event = (EventBuffer *)shmat(shmid, NULL, 0);
    if (event == (EventBuffer *)-1) {
        perror("shmat failed");
        return 1;
    }
    g_event = event;

    // Initialize flags
    event->has_event = 0;
    event->finished = 0;

    printf("=========================================\n");
    printf(" Event Monitor Running\n");
    printf(" Waiting for events from ./event_generator...\n");
    printf(" (Press Ctrl+C to exit)\n");
    printf("=========================================\n");

    while (1) {
        // If a new event is available, display it
        if (event->has_event == 1) {
            printf("\n[EVENT RECEIVED]\n");
            printf("  ID      : %d\n", event->id);
            printf("  Type    : %s\n", event->type);
            printf("  Message : %s\n", event->message);

            // Mark event as consumed
            event->has_event = 0;
        }

        // Exit if generator finished
        if (event->finished == 1 && event->has_event == 0) {
            printf("\nGenerator signaled exit. Closing monitor.\n");
            break;
        }

        usleep(50000); // Check every 50ms
    }

    // Clean up
    shmdt(event);
    shmctl(shmid, IPC_RMID, NULL);
    printf("Shared memory cleaned up successfully.\n");
    return 0;
}
