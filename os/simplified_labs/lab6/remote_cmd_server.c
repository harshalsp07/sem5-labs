/*
 * Lab 6: Remote Command Execution Server
 *
 * Purpose:
 *   Listens on port 8082.
 *   Receives a command string from the client (e.g. "ls -l" or "uname -a").
 *   Executes the command locally using popen() and sends the output
 *   back to the client.
 *
 * Compilation:
 *   gcc remote_cmd_server.c -o remote_cmd_server
 *
 * Usage:
 *   ./remote_cmd_server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8082
#define BUFFER_SIZE 2048

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addr_len = sizeof(address);
    char command[BUFFER_SIZE];
    char output_buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    listen(server_fd, 5);
    printf("====================================================\n");
    printf(" Remote Command Server listening on port %d...\n", PORT);
    printf("====================================================\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addr_len);
        if (client_fd < 0) continue;

        printf("Client connected for remote command execution.\n");

        while (1) {
            memset(command, 0, sizeof(command));
            int bytes_read = recv(client_fd, command, sizeof(command) - 1, 0);
            if (bytes_read <= 0) {
                printf("Client disconnected.\n");
                break;
            }

            command[strcspn(command, "\r\n")] = 0; // Strip newline

            if (strcmp(command, "exit") == 0) {
                break;
            }

            printf("Executing remote command: \"%s\"\n", command);

            // Execute command and capture output
            FILE *cmd_pipe = popen(command, "r");
            if (!cmd_pipe) {
                char err_msg[] = "Failed to execute command on server.\n";
                send(client_fd, err_msg, strlen(err_msg), 0);
                continue;
            }

            // Read output and send to client
            while (fgets(output_buffer, sizeof(output_buffer), cmd_pipe) != NULL) {
                send(client_fd, output_buffer, strlen(output_buffer), 0);
            }

            pclose(cmd_pipe);

            // Send end-of-output marker so client knows execution finished
            char eof_marker[] = "\n---END---\n";
            send(client_fd, eof_marker, strlen(eof_marker), 0);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
