#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <libgen.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_FILENAME 255

#define CMD_UPLOAD 1
#define CMD_DOWNLOAD 2
#define CMD_EXIT 3

#define STATUS_OK 0
#define STATUS_ERROR 1

#define SERVER_STORAGE_DIR "server_storage"

/* Receive exactly 'length' bytes */
int recv_all(int sock, void *buffer, size_t length)
{
    size_t total = 0;
    char *ptr = (char *)buffer;

    while (total < length)
    {
        ssize_t bytes = recv(sock, ptr + total, length - total, 0);
        if (bytes <= 0)
        {
            return -1;
        }
        total += bytes;
    }
    return 0;
}

/* Send exactly 'length' bytes */
int send_all(int sock, const void *buffer, size_t length)
{
    size_t total = 0;
    const char *ptr = (const char *)buffer;

    while (total < length)
    {
        ssize_t bytes = send(sock, ptr + total, length - total, 0);
        if (bytes <= 0)
        {
            return -1;
        }
        total += bytes;
    }
    return 0;
}

void handle_upload(int client_fd)
{
    uint32_t net_len;
    if (recv_all(client_fd, &net_len, sizeof(net_len)) < 0)
    {
        printf("Failed to receive filename length for upload.\n");
        return;
    }
    uint32_t filename_len = ntohl(net_len);

    if (filename_len == 0 || filename_len > MAX_FILENAME)
    {
        printf("Invalid filename length: %u\n", filename_len);
        return;
    }

    char filename[MAX_FILENAME + 1];
    if (recv_all(client_fd, filename, filename_len) < 0)
    {
        printf("Failed to receive filename.\n");
        return;
    }
    filename[filename_len] = '\0';

    uint64_t net_size;
    if (recv_all(client_fd, &net_size, sizeof(net_size)) < 0)
    {
        printf("Failed to receive file size.\n");
        return;
    }
    uint64_t file_size = be64toh(net_size);

    printf("\n[Upload Request] Filename: %s, Size: %lu bytes\n", filename, (unsigned long)file_size);

    // Save in SERVER_STORAGE_DIR
    mkdir(SERVER_STORAGE_DIR, 0777);
    char filepath[MAX_FILENAME + 64];
    snprintf(filepath, sizeof(filepath), "%s/%s", SERVER_STORAGE_DIR, basename(filename));

    FILE *fp = fopen(filepath, "wb");
    if (!fp)
    {
        perror("Failed to open file for writing on server");
        uint32_t status = htonl(STATUS_ERROR);
        send_all(client_fd, &status, sizeof(status));
        return;
    }

    char buffer[BUFFER_SIZE];
    uint64_t total_received = 0;
    int success = 1;

    while (total_received < file_size)
    {
        size_t to_recv = BUFFER_SIZE;
        uint64_t remaining = file_size - total_received;
        if (remaining < BUFFER_SIZE)
        {
            to_recv = (size_t)remaining;
        }

        ssize_t bytes_in = recv(client_fd, buffer, to_recv, 0);
        if (bytes_in <= 0)
        {
            printf("Connection error during upload.\n");
            success = 0;
            break;
        }

        if (fwrite(buffer, 1, bytes_in, fp) != (size_t)bytes_in)
        {
            printf("Error writing to file on server.\n");
            success = 0;
            break;
        }

        total_received += bytes_in;
    }

    fclose(fp);

    uint32_t status;
    if (success)
    {
        printf("File '%s' uploaded successfully and saved to '%s'.\n", filename, filepath);
        status = htonl(STATUS_OK);
    }
    else
    {
        remove(filepath);
        status = htonl(STATUS_ERROR);
    }
    send_all(client_fd, &status, sizeof(status));
}

void handle_download(int client_fd)
{
    uint32_t net_len;
    if (recv_all(client_fd, &net_len, sizeof(net_len)) < 0)
    {
        printf("Failed to receive filename length for download.\n");
        return;
    }
    uint32_t filename_len = ntohl(net_len);

    if (filename_len == 0 || filename_len > MAX_FILENAME)
    {
        printf("Invalid filename length: %u\n", filename_len);
        uint32_t status = htonl(STATUS_ERROR);
        send_all(client_fd, &status, sizeof(status));
        return;
    }

    char filename[MAX_FILENAME + 1];
    if (recv_all(client_fd, filename, filename_len) < 0)
    {
        printf("Failed to receive filename.\n");
        return;
    }
    filename[filename_len] = '\0';

    printf("\n[Download Request] Filename: %s\n", filename);

    // Look for file in SERVER_STORAGE_DIR, or in current working directory
    char filepath[MAX_FILENAME + 64];
    snprintf(filepath, sizeof(filepath), "%s/%s", SERVER_STORAGE_DIR, basename(filename));

    FILE *fp = fopen(filepath, "rb");
    if (!fp)
    {
        // Try fallback to filename directly
        fp = fopen(filename, "rb");
        if (!fp)
        {
            printf("File '%s' not found on server.\n", filename);
            uint32_t status = htonl(STATUS_ERROR);
            send_all(client_fd, &status, sizeof(status));
            return;
        }
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size < 0)
    {
        fclose(fp);
        uint32_t status = htonl(STATUS_ERROR);
        send_all(client_fd, &status, sizeof(status));
        return;
    }
    fseek(fp, 0, SEEK_SET);

    uint64_t file_size = (uint64_t)size;

    // Send STATUS_OK
    uint32_t status = htonl(STATUS_OK);
    if (send_all(client_fd, &status, sizeof(status)) < 0)
    {
        fclose(fp);
        return;
    }

    // Send file size
    uint64_t net_size = htobe64(file_size);
    if (send_all(client_fd, &net_size, sizeof(net_size)) < 0)
    {
        fclose(fp);
        return;
    }

    printf("Sending file '%s' (%lu bytes)...\n", filename, (unsigned long)file_size);

    char buffer[BUFFER_SIZE];
    uint64_t total_sent = 0;

    while (total_sent < file_size)
    {
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, fp);
        if (bytes_read > 0)
        {
            if (send_all(client_fd, buffer, bytes_read) < 0)
            {
                printf("Error sending file data.\n");
                break;
            }
            total_sent += bytes_read;
        }
        if (bytes_read < BUFFER_SIZE)
        {
            if (feof(fp)) break;
            if (ferror(fp))
            {
                printf("Error reading file.\n");
                break;
            }
        }
    }

    fclose(fp);
    printf("File '%s' sent successfully (%lu bytes).\n", filename, (unsigned long)total_sent);
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    mkdir(SERVER_STORAGE_DIR, 0777);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Bind successful on port %d.\n", PORT);

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("\nClient connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // Persistent loop for multiple operations
        while (1)
        {
            uint32_t net_cmd;
            if (recv_all(client_fd, &net_cmd, sizeof(net_cmd)) < 0)
            {
                printf("Client disconnected or connection lost.\n");
                break;
            }

            uint32_t cmd = ntohl(net_cmd);

            if (cmd == CMD_EXIT)
            {
                printf("Client requested clean Exit.\n");
                break;
            }
            else if (cmd == CMD_UPLOAD)
            {
                handle_upload(client_fd);
            }
            else if (cmd == CMD_DOWNLOAD)
            {
                handle_download(client_fd);
            }
            else
            {
                printf("Unknown command: %u\n", cmd);
                uint32_t status = htonl(STATUS_ERROR);
                send_all(client_fd, &status, sizeof(status));
            }
        }

        close(client_fd);
        printf("Client connection closed. Waiting for new clients...\n");
    }

    close(server_fd);
    return 0;
}
