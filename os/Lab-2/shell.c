#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_INPUT_LEN 1024
#define MAX_ARGS 64

/*
 * Reaps any terminated background processes without blocking.
 * Prints "Shell: Background process finished" for each reaped process.
 */
void reap_background_processes(void) {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Shell: Background process finished\n");
        fflush(stdout);
    }
}

/*
 * Parses the input line into an array of arguments (args).
 * Detects if the last argument is "&" for background execution.
 * Returns the number of arguments (excluding trailing NULL and "&").
 */
int parse_line(char *line, char **args, int *is_background) {
    int count = 0;
    char *token = strtok(line, " \t\r\n");
    while (token != NULL && count < MAX_ARGS - 1) {
        args[count++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    args[count] = NULL;
    *is_background = 0;

    if (count > 0 && strcmp(args[count - 1], "&") == 0) {
        *is_background = 1;
        args[count - 1] = NULL;
        count--;
    }
    return count;
}

int main(void) {
    char line[MAX_INPUT_LEN];
    char *args[MAX_ARGS];
    int is_background = 0;

    while (1) {
        // Reap any terminated background processes before prompting
        reap_background_processes();

        // Display prompt
        printf("shell> ");
        fflush(stdout);

        // Read user input
        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (feof(stdin)) {
                // EOF (Ctrl+D)
                printf("\n");
                break;
            }
            if (ferror(stdin) && errno == EINTR) {
                clearerr(stdin);
                printf("\n");
                continue;
            }
            break;
        }

        // Parse line into arguments
        int num_args = parse_line(line, args, &is_background);
        if (num_args == 0) {
            continue;
        }

        // Handle shell built-in commands
        if (strcmp(args[0], "exit") == 0) {
            reap_background_processes();
            break;
        }

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL || strcmp(args[1], "~") == 0) {
                char *home = getenv("HOME");
                if (home != NULL) {
                    if (chdir(home) != 0) {
                        perror("cd failed");
                    }
                }
            } else {
                if (chdir(args[1]) != 0) {
                    perror("cd failed");
                }
            }
            continue;
        }

        // Fork child process for command execution
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
        } else if (pid == 0) {
            // Child Process: execute command
            if (execvp(args[0], args) < 0) {
                perror("Execution failed");
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent Process
            if (is_background) {
                // Background execution: do not wait, prompt user immediately
            } else {
                // Foreground execution: wait specifically for this foreground child PID
                int status;
                while (waitpid(pid, &status, 0) == -1) {
                    if (errno != EINTR) {
                        perror("waitpid failed");
                        break;
                    }
                }
                // Reap any background processes that completed during foreground execution
                reap_background_processes();
            }
        }
    }

    return 0;
}