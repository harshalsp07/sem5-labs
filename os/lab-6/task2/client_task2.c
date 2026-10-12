#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <libgen.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_FILENAME 255

#define CMD_UPLOAD 1
#define CMD_DOWNLOAD 2
#define CMD_EXIT 3

#define STATUS_OK 0
#define STATUS_ERROR 1

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

void upload_file(int sock)
{
    char filepath[MAX_FILENAME + 1];
    printf("Enter filename to upload: ");
    if (scanf("%255s", filepath) != 1) return;

    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        perror("Failed to open local file");
        return;
    }

    // Determine file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size < 0)
    {
        perror("ftell failed");
        fclose(file);
        return;
    }
    fseek(file, 0, SEEK_SET);

    uint64_t file_size = (uint64_t)size;
    char *base_name = basename(filepath);
    uint32_t filename_len = (uint32_t)strlen(base_name);

    // Send command
    uint32_t net_cmd = htonl(CMD_UPLOAD);
    if (send_all(sock, &net_cmd, sizeof(net_cmd)) < 0)
    {
        printf("Failed to send command.\n");
        fclose(file);
        return;
    }

    // Send filename length
    uint32_t net_filename_len = htonl(filename_len);
    if (send_all(sock, &net_filename_len, sizeof(net_filename_len)) < 0)
    {
        printf("Failed to send filename length.\n");
        fclose(file);
        return;
    }

    // Send filename
    if (send_all(sock, base_name, filename_len) < 0)
    {
        printf("Failed to send filename.\n");
        fclose(file);
        return;
    }

    // Send file size
    uint64_t net_file_size = htobe64(file_size);
    if (send_all(sock, &net_file_size, sizeof(net_file_size)) < 0)
    {
        printf("Failed to send file size.\n");
        fclose(file);
        return;
    }

    printf("Sending file '%s' (%lu bytes)...\n", base_name, (unsigned long)file_size);

    char buffer[BUFFER_SIZE];
    uint64_t total_sent = 0;

    while (total_sent < file_size)
    {
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
        if (bytes_read > 0)
        {
            if (send_all(sock, buffer, bytes_read) < 0)
            {
                printf("Error sending file data.\n");
                fclose(file);
                return;
            }
            total_sent += bytes_read;
            printf("\rUploading: %lu / %lu bytes (%.1f%%)",
                   (unsigned long)total_sent,
                   (unsigned long)file_size,
                   (double)total_sent * 100.0 / (file_size ? file_size : 1));
            fflush(stdout);
        }
        if (bytes_read < BUFFER_SIZE)
        {
            if (feof(file)) break;
            if (ferror(file))
            {
                printf("\nError reading file.\n");
                fclose(file);
                return;
            }
        }
    }

    fclose(file);
    printf("\nFile data transmitted.\n");

    // Receive server acknowledgment
    uint32_t net_status;
    if (recv_all(sock, &net_status, sizeof(net_status)) < 0)
    {
        printf("Failed to receive status acknowledgment.\n");
        return;
    }
    uint32_t status = ntohl(net_status);

    if (status == STATUS_OK)
    {
        printf("Server confirmed: File uploaded successfully!\n");
    }
    else
    {
        printf("Server reported an error during upload.\n");
    }
}

void download_file(int sock)
{
    char filename[MAX_FILENAME + 1];
    printf("Enter filename to download: ");
    if (scanf("%255s", filename) != 1) return;

    uint32_t filename_len = (uint32_t)strlen(filename);

    // Send command
    uint32_t net_cmd = htonl(CMD_DOWNLOAD);
    if (send_all(sock, &net_cmd, sizeof(net_cmd)) < 0)
    {
        printf("Failed to send command.\n");
        return;
    }

    // Send filename length
    uint32_t net_filename_len = htonl(filename_len);
    if (send_all(sock, &net_filename_len, sizeof(net_filename_len)) < 0)
    {
        printf("Failed to send filename length.\n");
        return;
    }

    // Send filename
    if (send_all(sock, filename, filename_len) < 0)
    {
        printf("Failed to send filename.\n");
        return;
    }

    // Receive server status
    uint32_t net_status;
    if (recv_all(sock, &net_status, sizeof(net_status)) < 0)
    {
        printf("Connection lost while waiting for server response.\n");
        return;
    }
    uint32_t status = ntohl(net_status);

    if (status != STATUS_OK)
    {
        printf("Server error: File '%s' not found or cannot be read on server.\n", filename);
        return;
    }

    // Receive file size
    uint64_t net_file_size;
    if (recv_all(sock, &net_file_size, sizeof(net_file_size)) < 0)
    {
        printf("Failed to receive file size.\n");
        return;
    }
    uint64_t file_size = be64toh(net_file_size);

    printf("File size on server: %lu bytes. Starting download...\n", (unsigned long)file_size);

    char out_filename[MAX_FILENAME + 32];
    snprintf(out_filename, sizeof(out_filename), "downloaded_%s", filename);

    FILE *out_fp = fopen(out_filename, "wb");
    if (!out_fp)
    {
        perror("Failed to create local destination file");
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

        ssize_t bytes_in = recv(sock, buffer, to_recv, 0);
        if (bytes_in <= 0)
        {
            printf("\nConnection lost during download.\n");
            success = 0;
            break;
        }

        if (fwrite(buffer, 1, bytes_in, out_fp) != (size_t)bytes_in)
        {
            printf("\nError writing to local file.\n");
            success = 0;
            break;
        }

        total_received += bytes_in;
        printf("\rDownloading: %lu / %lu bytes (%.1f%%)",
               (unsigned long)total_received,
               (unsigned long)file_size,
               (double)total_received * 100.0 / (file_size ? file_size : 1));
        fflush(stdout);
    }

    fclose(out_fp);

    if (success)
    {
        printf("\nFile downloaded successfully and saved as: %s\n", out_filename);
    }
    else
    {
        remove(out_filename);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("Invalid server IP address");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Connected to File Transfer Server.\n");

    while (1)
    {
        printf("\n====================================\n");
        printf("        FILE TRANSFER SYSTEM        \n");
        printf("====================================\n");
        printf("1. Upload File\n");
        printf("2. Download File\n");
        printf("3. Exit\n");
        printf("====================================\n");
        printf("Enter your choice: ");

        int choice;
        if (scanf("%d", &choice) != 1)
        {
            // Clear invalid input
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter 1, 2, or 3.\n");
            continue;
        }

        if (choice == 1)
        {
            upload_file(sock);
        }
        else if (choice == 2)
        {
            download_file(sock);
        }
        else if (choice == 3)
        {
            uint32_t net_cmd = htonl(CMD_EXIT);
            send_all(sock, &net_cmd, sizeof(net_cmd));
            printf("Exiting File Transfer System...\n");
            break;
        }
        else
        {
            printf("Invalid choice! Please select 1, 2, or 3.\n");
        }
    }

    close(sock);
    return 0;
}
