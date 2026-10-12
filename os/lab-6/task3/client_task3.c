#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUF 4096

void send_all(int s, const void *buf, int n)
{
    int sent = 0;
    const char *ptr = (const char *)buf;
    while (sent < n)
    {
        int bytes = send(s, ptr + sent, n - sent, 0);
        if (bytes <= 0) break;
        sent += bytes;
    }
}

int recv_all(int s, void *buf, int n)
{
    int rec = 0;
    char *ptr = (char *)buf;
    while (rec < n)
    {
        int bytes = recv(s, ptr + rec, n - rec, 0);
        if (bytes <= 0) return -1;
        rec += bytes;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char filename[256];
    char buffer[BUF];

    if (argc != 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &server.sin_addr) <= 0)
    {
        perror("Invalid server IP");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Connected to server.\n");

    printf("Enter C source filename: ");
    if (scanf("%255s", filename) != 1)
    {
        close(sock);
        return 1;
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        perror("Failed to open file");
        close(sock);
        return 1;
    }

    // Send filename length & filename
    int name_len = strlen(filename);
    send_all(sock, &name_len, sizeof(name_len));
    send_all(sock, filename, name_len);

    // Determine and send file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    send_all(sock, &file_size, sizeof(file_size));

    // Send source file data
    printf("Sending file...\n");
    long sent = 0;
    while (sent < file_size)
    {
        int n = fread(buffer, 1, BUF, fp);
        if (n <= 0) break;
        send_all(sock, buffer, n);
        sent += n;
    }
    fclose(fp);

    printf("File sent.\n\n");
    printf("Waiting for execution result...\n\n");

    // Receive execution type (0 = compile error, 1 = execution output)
    int type = 0;
    if (recv_all(sock, &type, sizeof(type)) < 0)
    {
        printf("Failed to receive execution status from server.\n");
        close(sock);
        return 1;
    }

    if (type == 0)
    {
        int len = 0;
        recv_all(sock, &len, sizeof(len));
        char *error = malloc(len + 1);
        if (error)
        {
            recv_all(sock, error, len);
            error[len] = '\0';
            printf("Compilation failed:\n");
            printf("--------------------------------\n");
            printf("%s", error);
            printf("--------------------------------\n");
            free(error);
        }
    }
    else
    {
        int len = 0;
        recv_all(sock, &len, sizeof(len));
        char *output = malloc(len + 1);
        if (output)
        {
            recv_all(sock, output, len);
            output[len] = '\0';
            printf("Program Output:\n");
            printf("--------------------------------\n");
            printf("%s", output);
            printf("--------------------------------\n");
            free(output);
        }
    }

    close(sock);
    return 0;
}
