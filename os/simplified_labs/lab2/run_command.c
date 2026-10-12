/*
 * Lab 2: Running Commands with fork() and execvp()
 *
 * Purpose:
 *   Demonstrates executing an external system program in a child process
 *   using execvp(). The parent waits until the command completes.
 *
 * Compilation:
 *   gcc run_command.c -o run_command
 *
 * Usage:
 *   ./run_command ls -l
 *   ./run_command date
 *   ./run_command whoami
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command> [arguments...]\n", argv[0]);
        printf("Example: %s ls -l\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process: execute the specified command
        // &argv[1] passes the command name and its parameters
        execvp(argv[1], &argv[1]);

        // If execvp returns, it means an error occurred
        perror("execvp failed");
        exit(1);
    } else {
        // Parent process: wait for child to finish
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("\n[Parent] Command '%s' finished with exit status: %d\n", argv[1], WEXITSTATUS(status));
        }
    }

    return 0;
}
