/*
 * Lab 6: Simple TCP Calculator Client
 *
 * Purpose:
 *   Connects to the Calculator Server on 127.0.0.1:8080.
 *   Sends arithmetic expressions (e.g. "12 + 8") and receives results.
 *
 * Compilation:
 *   gcc calc_client.c -o calc_client
 *
 * Usage:
 *   ./calc_client
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    // 1. Create TCP socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // 2. Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // 3. Connect to server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed (Make sure ./calc_server is running)");
        return 1;
    }

    printf("=========================================\n");
    printf(" Connected to Calculator Server!\n");
    printf(" Format : <number> <operator> <number>\n");
    printf(" Example: 25 * 4\n");
    printf(" Type 'exit' to quit.\n");
    printf("=========================================\n");

    while (1) {
        printf("calc> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Send to server
        send(sock_fd, input, strlen(input), 0);

        if (strncmp(input, "exit", 4) == 0) {
            printf("Exiting calculator client.\n");
            break;
        }

        // Receive result from server
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            printf("Server closed connection.\n");
            break;
        }

        printf("%s", buffer);
    }

    close(sock_fd);
    return 0;
}
