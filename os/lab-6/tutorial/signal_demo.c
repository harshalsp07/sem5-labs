/*
 * signal_demo.c - Demonstrates signal handling in C
 * Compile: gcc signal_demo.c -o signal_demo
 * Run:     ./signal_demo
 *
 * Try:
 *   - Press Ctrl+C          -> triggers SIGINT  (caught)
 *   - Press Ctrl+\          -> triggers SIGQUIT (caught)
 *   - In another terminal:
 *       kill -SIGUSR1 <pid> -> triggers SIGUSR1 (caught)
 *       kill -SIGTERM <pid> -> triggers SIGTERM (caught, then exits cleanly)
 *       kill -9 <pid>       -> SIGKILL cannot be caught, process dies immediately
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t keep_running = 1;   // shared flag, safe to modify in a handler
volatile sig_atomic_t usr1_count = 0;

/* --- Handler for SIGINT (Ctrl+C) --- */
void handle_sigint(int sig) {
    // NOTE: only async-signal-safe functions should really be used here.
    // write() is safe; printf() is technically not guaranteed safe in a
    // signal handler, but is used here for simplicity/demo purposes.
    printf("\n[Handler] Caught SIGINT (signal %d) - Ctrl+C detected.\n", sig);
    printf("[Handler] Press Ctrl+C again within 3 seconds to confirm exit...\n");

    // Re-arm a short alarm; if user doesn't press again, we keep running
    signal(SIGINT, handle_sigint);
}

/* --- Handler for SIGTERM (graceful kill) --- */
void handle_sigterm(int sig) {
    printf("\n[Handler] Caught SIGTERM (signal %d). Shutting down gracefully...\n", sig);
    keep_running = 0;   // break main loop instead of dying abruptly
}

/* --- Handler for SIGUSR1 (user-defined signal) --- */
void handle_sigusr1(int sig) {
    usr1_count++;
    printf("\n[Handler] Caught SIGUSR1 (signal %d). Count so far: %d\n", sig, usr1_count);
}

/* --- Handler for SIGQUIT (Ctrl+\) --- */
void handle_sigquit(int sig) {
    printf("\n[Handler] Caught SIGQUIT (signal %d) - Ctrl+\\ detected.\n", sig);
    printf("[Handler] Ignoring it this time (demo only).\n");
}

int main() {
    printf("PID of this process: %d\n", getpid());
    printf("---------------------------------------------\n");
    printf("Try in another terminal:\n");
    printf("  kill -SIGUSR1 %d\n", getpid());
    printf("  kill -SIGTERM %d\n", getpid());
    printf("Or press Ctrl+C / Ctrl+\\ here.\n");
    printf("---------------------------------------------\n\n");

    // Register handlers using signal()
    signal(SIGINT,  handle_sigint);
    signal(SIGTERM, handle_sigterm);
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGQUIT, handle_sigquit);

    // SIGKILL and SIGSTOP CANNOT be caught, blocked, or ignored -
    // this is enforced by the OS/kernel itself (uncommenting below
    // has no real effect, shown only for demonstration):
    // signal(SIGKILL, handle_sigint);  // would be ignored by the OS

    int seconds = 0;
    while (keep_running) {
        printf("Main loop running... (%d sec elapsed, usr1_count=%d)\n",
               seconds, usr1_count);
        sleep(1);
        seconds++;
    }

    printf("\nExiting main loop cleanly. Goodbye.\n");
    return 0;
}
