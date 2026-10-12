/*
 * Lab 2: Simple Unix Shell
 *
 * Features:
 *   - Runs interactive commands (e.g. ls, pwd, cat, date).
 *   - Built-in command: 'cd <directory>' to change folders.
 *   - Built-in command: 'exit' to quit the shell.
 *   - Background execution: appending '&' (e.g. sleep 5 &).
 *
 * Compilation:
 *   gcc simple_shell.c -o simple_shell
 *
 * Usage:
 *   ./simple_shell
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    printf("=========================================\n");
    printf(" Welcome to Simple Shell!\n");
    printf(" Type commands (or 'exit' to quit)\n");
    printf(" Add '&' at the end to run in background\n");
    printf("=========================================\n");

    while (1) {
        // 1. Clean up any completed background processes
        while (waitpid(-1, NULL, WNOHANG) > 0) {
            // Reaped background child
        }

        // 2. Display shell prompt
        printf("simple_shell> ");
        fflush(stdout);

        // 3. Read input line
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\nExiting shell.\n");
            break;
        }

        // 4. Split input into argument tokens
        int count = 0;
        char *token = strtok(line, " \t\r\n");
        while (token != NULL && count < MAX_ARGS - 1) {
            args[count++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        args[count] = NULL;

        // Skip empty input lines
        if (count == 0) {
            continue;
        }

        // 5. Check for background operator '&'
        int is_background = 0;
        if (strcmp(args[count - 1], "&") == 0) {
            is_background = 1;
            args[count - 1] = NULL; // Remove '&' before executing
            count--;
            if (count == 0) continue;
        }

        // 6. Handle built-in command: exit
        if (strcmp(args[0], "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }

        // 7. Handle built-in command: cd
        if (strcmp(args[0], "cd") == 0) {
            char *target_dir = args[1];
            if (target_dir == NULL) {
                target_dir = getenv("HOME");
            }
            if (chdir(target_dir) != 0) {
                perror("cd failed");
            }
            continue;
        }

        // 8. Fork a child process to run external command
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
        } else if (pid == 0) {
            // Child process executes the command
            execvp(args[0], args);
            perror("Command execution failed");
            exit(1);
        } else {
            // Parent process
            if (is_background) {
                printf("[Started background PID %d]\n", pid);
            } else {
                // Wait for foreground child to finish
                waitpid(pid, NULL, 0);
            }
        }
    }

    return 0;
}
