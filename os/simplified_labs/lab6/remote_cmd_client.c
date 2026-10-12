/*
 * Lab 6: Remote Command Execution Client
 *
 * Purpose:
 *   Connects to port 8082 on the server.
 *   Sends commands typed by the user, and prints the remote output.
 *
 * Compilation:
 *   gcc remote_cmd_client.c -o remote_cmd_client
 *
 * Usage:
 *   ./remote_cmd_client
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8082
#define BUFFER_SIZE 2048

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed (Make sure ./remote_cmd_server is running)");
        return 1;
    }

    printf("====================================================\n");
    printf(" Connected to Remote Command Server!\n");
    printf(" Enter commands to execute on the server (e.g. ls, uname -a, date)\n");
    printf(" Type 'exit' to disconnect.\n");
    printf("====================================================\n");

    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    while (1) {
        printf("remote> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) break;

        send(sock_fd, command, strlen(command), 0);

        if (strncmp(command, "exit", 4) == 0) {
            printf("Exiting remote command client.\n");
            break;
        }

        // Receive output until "---END---"
        while (1) {
            memset(buffer, 0, sizeof(buffer));
            int bytes = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes <= 0) {
                printf("Server disconnected.\n");
                close(sock_fd);
                return 0;
            }

            char *marker = strstr(buffer, "\n---END---\n");
            if (marker != NULL) {
                *marker = '\0';
                printf("%s\n", buffer);
                break;
            } else {
                printf("%s", buffer);
            }
        }
    }

    close(sock_fd);
    return 0;
}
