#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define PORT 8080
#define BUF 4096

void send_all(int s, void *buf, int n)
{
    int sent = 0;

    while (sent < n)
        sent += send(s, buf + sent, n - sent, 0);
}

void recv_all(int s, void *buf, int n)
{
    int rec = 0;

    while (rec < n)
        rec += recv(s, buf + rec, n - rec, 0);
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

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    inet_pton(AF_INET,
              argv[1],
              &server.sin_addr);

    /* Connect */
    connect(sock,
            (struct sockaddr *)&server,
            sizeof(server));

    printf("Connected to server.\n");

    /* Get C file */
    printf("Enter C source filename: ");
    scanf("%255s", filename);

    FILE *fp = fopen(filename, "rb");

    if (!fp)
    {
        perror("File");
        close(sock);
        return 1;
    }

    /* Filename */
    int name_len = strlen(filename);

    send_all(sock,
             &name_len,
             sizeof(name_len));

    send_all(sock,
             filename,
             name_len);

    /* File size */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    send_all(sock,
             &file_size,
             sizeof(file_size));

    /* Send source file */
    printf("Sending file...\n");

    long sent = 0;

    while (sent < file_size)
    {
        int n = fread(buffer, 1, BUF, fp);

        send_all(sock, buffer, n);

        sent += n;

        printf("\rSent %ld / %ld bytes",
               sent, file_size);
    }

    fclose(fp);

    printf("\nFile sent.\n");
    printf("Waiting for compilation...\n");

    /* Receive result type */
    int type;

    recv_all(sock,
             &type,
             sizeof(type));

    /* Compilation error */
    if (type == 0)
    {
        int len;

        recv_all(sock,
                 &len,
                 sizeof(len));

        char *error = malloc(len + 1);

        recv_all(sock,
                 error,
                 len);

        error[len] = '\0';

        printf("\nCompilation failed:\n");
        printf("--------------------------------\n");
        printf("%s", error);
        printf("--------------------------------\n");

        free(error);
    }

    /* Compilation successful */
    else
    {
        long exe_size;

        recv_all(sock,
                 &exe_size,
                 sizeof(exe_size));

        printf("\nCompilation successful!\n");
        printf("Executable size: %ld bytes\n",
               exe_size);

        fp = fopen("downloaded_program", "wb");

        long received = 0;

        while (received < exe_size)
        {
            int n = (exe_size - received > BUF)
                    ? BUF
                    : exe_size - received;

            recv_all(sock,
                     buffer,
                     n);

            fwrite(buffer, 1, n, fp);

            received += n;

            printf("\rReceived %ld / %ld bytes",
                   received, exe_size);
        }

        fclose(fp);

        printf("\n\nExecutable received!\n");
        printf("Saved as: downloaded_program\n");

        chmod("downloaded_program", 0755);

        printf("\nRun it using:\n");
        printf("./downloaded_program\n");
    }

    close(sock);

    return 0;
}
