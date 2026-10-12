/*
 * Lab 6: Simple TCP File Transfer Server
 *
 * Purpose:
 *   Listens on port 8081.
 *   Receives a filename followed by file data from a client,
 *   and saves it as "received_<filename>".
 *
 * Compilation:
 *   gcc file_server.c -o file_server
 *
 * Usage:
 *   ./file_server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addr_len = sizeof(address);
    char buffer[BUFFER_SIZE];

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
    printf("=========================================\n");
    printf(" File Transfer Server listening on port %d...\n", PORT);
    printf("=========================================\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addr_len);
        if (client_fd < 0) continue;

        printf("Client connected for file transfer.\n");

        // 1. Receive filename
        char filename[256] = {0};
        int name_len = recv(client_fd, filename, sizeof(filename) - 1, 0);
        if (name_len <= 0) {
            close(client_fd);
            continue;
        }

        char out_filename[300];
        snprintf(out_filename, sizeof(out_filename), "received_%s", filename);
        printf("Saving incoming file as: %s\n", out_filename);

        FILE *fp = fopen(out_filename, "wb");
        if (!fp) {
            perror("Error opening file to write");
            close(client_fd);
            continue;
        }

        // Send ready acknowledgement
        send(client_fd, "READY", 5, 0);

        // 2. Receive file content chunks
        int bytes_received;
        long total_bytes = 0;
        while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
            fwrite(buffer, 1, bytes_received, fp);
            total_bytes += bytes_received;
        }

        fclose(fp);
        close(client_fd);
        printf("Transfer complete! Received %ld bytes into '%s'.\n", total_bytes, out_filename);
    }

    close(server_fd);
    return 0;
}
