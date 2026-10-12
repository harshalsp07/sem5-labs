/*
 * Lab 5 Solutions - Problem 3: Bidirectional Pipe Message Service
 *
 * Purpose:
 *   Demonstrates full two-way communication between Parent and Child using two pipes:
 *     - Pipe 1 (p2c): Parent sends messages to Child.
 *     - Pipe 2 (c2p): Child processes the message (converts to UPPERCASE)
 *                     and sends the response back to Parent.
 *
 * Compilation:
 *   gcc message_service.c -o message_service
 *
 * Usage:
 *   ./message_service
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main() {
    int p2c[2]; // Parent-to-Child pipe
    int c2p[2]; // Child-to-Parent pipe

    // 1. Create two pipes
    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid > 0) {
        // --- PARENT PROCESS ---
        close(p2c[0]); // Close read end of p2c
        close(c2p[1]); // Close write end of c2p

        char send_buf[BUFFER_SIZE];
        char recv_buf[BUFFER_SIZE];

        printf("=========================================\n");
        printf(" Two-Way Pipe Message Service\n");
        printf(" Type a message (or 'exit' to quit)\n");
        printf("=========================================\n");

        while (1) {
            printf("\nParent> ");
            fflush(stdout);

            if (fgets(send_buf, sizeof(send_buf), stdin) == NULL) break;
            send_buf[strcspn(send_buf, "\r\n")] = 0; // Remove newline

            if (strlen(send_buf) == 0) continue;

            // Send to child
            write(p2c[1], send_buf, strlen(send_buf) + 1);

            if (strcmp(send_buf, "exit") == 0) {
                printf("Parent exiting.\n");
                break;
            }

            // Read response from child
            read(c2p[0], recv_buf, sizeof(recv_buf));
            printf("Child reply (processed): %s\n", recv_buf);
        }

        close(p2c[1]);
        close(c2p[0]);
        wait(NULL); // Wait for child to exit
    } else {
        // --- CHILD PROCESS ---
        close(p2c[1]); // Close write end of p2c
        close(c2p[0]); // Close read end of c2p

        char buffer[BUFFER_SIZE];

        while (1) {
            int bytes = read(p2c[0], buffer, sizeof(buffer));
            if (bytes <= 0) break;

            if (strcmp(buffer, "exit") == 0) {
                break;
            }

            // Transform message to uppercase
            for (int i = 0; buffer[i]; i++) {
                buffer[i] = toupper((unsigned char)buffer[i]);
            }

            // Send processed reply back to parent
            write(c2p[1], buffer, strlen(buffer) + 1);
        }

        close(p2c[0]);
        close(c2p[1]);
        exit(0);
    }

    return 0;
}
