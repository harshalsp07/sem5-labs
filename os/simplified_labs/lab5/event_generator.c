/*
 * Lab 5 Solutions - Problem 1: Event Generator (Shared Memory)
 *
 * Purpose:
 *   Prompts the user to enter events (ID, Type, Message) and writes them
 *   into a shared memory segment for the Event Monitor to display.
 *
 * Compilation:
 *   gcc event_generator.c -o event_generator
 *
 * Usage:
 *   ./event_generator
 *   (Make sure to run ./event_monitor in another terminal first)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 1111

typedef struct {
    int id;
    char type[32];
    char message[128];
    int has_event; // 1 = new event available, 0 = processed
    int finished;  // 1 = generator has closed
} EventBuffer;

int main() {
    // 1. Connect to shared memory segment
    int shmid = shmget(SHM_KEY, sizeof(EventBuffer), 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }

    EventBuffer *event = (EventBuffer *)shmat(shmid, NULL, 0);
    if (event == (EventBuffer *)-1) {
        perror("shmat failed");
        return 1;
    }

    printf("=========================================\n");
    printf(" Event Generator Started\n");
    printf(" Enter Event ID (-1 to quit)\n");
    printf("=========================================\n");

    while (1) {
        int id;
        printf("\nEnter Event ID: ");
        if (scanf("%d", &id) != 1) break;

        if (id == -1) {
            event->finished = 1;
            printf("Exiting Event Generator.\n");
            break;
        }

        // Wait if previous event is still waiting to be read
        while (event->has_event == 1) {
            usleep(50000); // 50ms
        }

        event->id = id;

        printf("Enter Event Type (e.g. WARNING, INFO): ");
        scanf("%31s", event->type);

        printf("Enter Event Message: ");
        getchar(); // consume leftover newline
        fgets(event->message, sizeof(event->message), stdin);
        event->message[strcspn(event->message, "\r\n")] = 0; // Strip newline

        // Flag event as ready for monitor
        event->has_event = 1;
        printf("-> Event %d posted to shared memory!\n", id);
    }

    shmdt(event);
    return 0;
}
