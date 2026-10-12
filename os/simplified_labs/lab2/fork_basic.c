/*
 * Lab 2: Basic Process Creation with fork()
 *
 * Purpose:
 *   Demonstrates how fork() creates a new child process:
 *   - fork() returns 0 to the child process.
 *   - fork() returns the child's PID to the parent process.
 *   - The parent calls wait() to wait for the child to finish.
 *
 * Compilation:
 *   gcc fork_basic.c -o fork_basic
 *
 * Usage:
 *   ./fork_basic
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Original process started. PID = %d\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process code
        printf("[CHILD] Hello! I am the child process.\n");
        printf("[CHILD] My PID is: %d, My parent's PID is: %d\n", getpid(), getppid());
        sleep(1);
        printf("[CHILD] Child finishing now.\n");
        exit(0);
    } else {
        // Parent process code
        printf("[PARENT] I am the parent process. My PID is: %d\n", getpid());
        printf("[PARENT] Created child with PID: %d\n", pid);
        printf("[PARENT] Waiting for child to terminate...\n");

        wait(NULL); // Wait for child to exit

        printf("[PARENT] Child finished. Parent exiting.\n");
    }

    return 0;
}
