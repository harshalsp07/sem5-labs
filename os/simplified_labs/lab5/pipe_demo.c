/*
 * Lab 5: Inter-Process Communication using Anonymous Pipe
 *
 * Purpose:
 *   Demonstrates unidirectional communication between parent and child processes.
 *   - pipe(fd) creates two file descriptors:
 *       fd[0] is for reading.
 *       fd[1] is for writing.
 *
 * Compilation:
 *   gcc pipe_demo.c -o pipe_demo
 *
 * Usage:
 *   ./pipe_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2]; // fd[0] = read end, fd[1] = write end

    // 1. Create pipe
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    // 2. Fork child process
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid > 0) {
        // --- PARENT PROCESS (Writer) ---
        // Close unused read end
        close(fd[0]);

        char message[] = "Hello from Parent via Pipe!";
        printf("[PARENT] Sending message: \"%s\"\n", message);

        // Write message to pipe (+1 includes null terminator '\0')
        write(fd[1], message, strlen(message) + 1);

        // Close write end after writing
        close(fd[1]);

        // Wait for child to finish
        wait(NULL);
    } else {
        // --- CHILD PROCESS (Reader) ---
        // Close unused write end
        close(fd[1]);

        char buffer[100];

        // Read message from pipe
        read(fd[0], buffer, sizeof(buffer));
        printf("[CHILD]  Received message: \"%s\"\n", buffer);

        // Close read end after reading
        close(fd[0]);

        exit(0);
    }

    return 0;
}
