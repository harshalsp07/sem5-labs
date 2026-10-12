#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define STATE_RUNNING 0
#define STATE_PAUSED  1

volatile sig_atomic_t current_state = STATE_RUNNING;
volatile sig_atomic_t keep_running = 1;

/* Handler for SIGUSR1: pause main activity */
void handle_sigusr1(int sig)
{
    (void)sig;
    current_state = STATE_PAUSED;
    printf("\nSIGUSR1 received\nProcess paused\n");
    fflush(stdout);
    signal(SIGUSR1, handle_sigusr1);
}

/* Handler for SIGUSR2: resume main activity */
void handle_sigusr2(int sig)
{
    (void)sig;
    current_state = STATE_RUNNING;
    printf("\nSIGUSR2 received\nProcess resumed\n");
    fflush(stdout);
    signal(SIGUSR2, handle_sigusr2);
}

/* Handler for SIGINT (Ctrl+C): display warning and continue */
void handle_sigint(int sig)
{
    (void)sig;
    printf("\nCtrl+C pressed\nSIGINT received - process continues running\n");
    fflush(stdout);
    signal(SIGINT, handle_sigint);
}

/* Handler for SIGTERM: terminate cleanly */
void handle_sigterm(int sig)
{
    (void)sig;
    printf("\nSIGTERM received\nTerminating gracefully...\n");
    fflush(stdout);
    keep_running = 0;
}

int main()
{
    printf("Program started\nPID: %d\n", getpid());
    fflush(stdout);

    // Register signal handlers
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);
    signal(SIGINT,  handle_sigint);
    signal(SIGTERM, handle_sigterm);

    while (keep_running)
    {
        if (current_state == STATE_RUNNING)
        {
            printf("State: RUNNING\n");
        }
        else
        {
            printf("State: PAUSED\n");
        }
        fflush(stdout);
        sleep(1);
    }

    printf("Program exited\n");
    fflush(stdout);

    return 0;
}
