/*
 * Lab 6: Simple TCP File Transfer Client
 *
 * Purpose:
 *   Connects to File Transfer Server on port 8081.
 *   Sends the specified file across the network.
 *
 * Compilation:
 *   gcc file_client.c -o file_client
 *
 * Usage:
 *   ./file_client <filename_to_send>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <libgen.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filepath = argv[1];
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        perror("Error opening local file to send");
        return 1;
    }

    // 1. Create socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        fclose(fp);
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // 2. Connect to server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed (Make sure ./file_server is running)");
        fclose(fp);
        close(sock_fd);
        return 1;
    }

    // Extract basename of the file
    char path_copy[256];
    strncpy(path_copy, filepath, sizeof(path_copy) - 1);
    char *base_name = basename(path_copy);

    // 3. Send filename to server
    send(sock_fd, base_name, strlen(base_name), 0);

    // 4. Wait for server READY acknowledgement
    char ack[16] = {0};
    recv(sock_fd, ack, sizeof(ack) - 1, 0);

    // 5. Send file contents
    char buffer[BUFFER_SIZE];
    int bytes_read;
    long total_sent = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send(sock_fd, buffer, bytes_read, 0);
        total_sent += bytes_read;
    }

    fclose(fp);
    close(sock_fd);

    printf("File '%s' successfully sent (%ld bytes) to server.\n", base_name, total_sent);
    return 0;
}
