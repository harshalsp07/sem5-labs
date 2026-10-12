/*
 * Lab 6: Signal Handling in C
 *
 * Purpose:
 *   Demonstrates catching and handling UNIX signals:
 *   - SIGINT  (Ctrl+C)          : Displays warning message, process continues running.
 *   - SIGUSR1 (kill -SIGUSR1)   : Pauses the main activity.
 *   - SIGUSR2 (kill -SIGUSR2)   : Resumes the main activity.
 *   - SIGTERM (kill -SIGTERM)   : Gracefully terminates the process.
 *
 * Compilation:
 *   gcc signal_demo.c -o signal_demo
 *
 * Usage:
 *   ./signal_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define STATE_RUNNING 1
#define STATE_PAUSED  0

volatile sig_atomic_t state = STATE_RUNNING;
volatile sig_atomic_t keep_running = 1;

// Handler for Ctrl+C (SIGINT)
void handle_sigint(int sig) {
    (void)sig;
    printf("\n[Signal] Ctrl+C (SIGINT) caught! Process will NOT terminate.\n");
    signal(SIGINT, handle_sigint); // Re-arm handler
}

// Handler for SIGUSR1 (Pause)
void handle_sigusr1(int sig) {
    (void)sig;
    state = STATE_PAUSED;
    printf("\n[Signal] SIGUSR1 received -> State changed to PAUSED.\n");
    signal(SIGUSR1, handle_sigusr1);
}

// Handler for SIGUSR2 (Resume)
void handle_sigusr2(int sig) {
    (void)sig;
    state = STATE_RUNNING;
    printf("\n[Signal] SIGUSR2 received -> State changed to RUNNING.\n");
    signal(SIGUSR2, handle_sigusr2);
}

// Handler for SIGTERM (Graceful exit)
void handle_sigterm(int sig) {
    (void)sig;
    printf("\n[Signal] SIGTERM received -> Exiting gracefully.\n");
    keep_running = 0;
}

int main() {
    printf("====================================================\n");
    printf(" Signal Handling Demo (PID: %d)\n", getpid());
    printf(" Instructions:\n");
    printf("   1. Press Ctrl+C to test SIGINT interception.\n");
    printf("   2. In another terminal, run:\n");
    printf("        kill -SIGUSR1 %d   (to pause)\n", getpid());
    printf("        kill -SIGUSR2 %d   (to resume)\n", getpid());
    printf("        kill -SIGTERM %d   (to terminate cleanly)\n", getpid());
    printf("====================================================\n");

    // Register signal handlers
    signal(SIGINT,  handle_sigint);
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);
    signal(SIGTERM, handle_sigterm);

    int count = 0;
    while (keep_running) {
        if (state == STATE_RUNNING) {
            printf("[Tick %d] Status: RUNNING...\n", ++count);
        } else {
            printf("[Tick %d] Status: PAUSED (Waiting for SIGUSR2)...\n", ++count);
        }
        sleep(1);
    }

    printf("Cleanup complete. Program ended.\n");
    return 0;
}
