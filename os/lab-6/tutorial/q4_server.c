#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

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

int main()
{
    int server, client;
    struct sockaddr_in addr;
    char filename[256];
    char buffer[BUF];

    /* Create socket */
    server = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, 5);

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        client = accept(server, NULL, NULL);
        printf("\nClient connected.\n");

        /* Receive filename */
        int name_len;
        recv_all(client, &name_len, sizeof(name_len));
        recv_all(client, filename, name_len);
        filename[name_len] = '\0';

        /* Receive file size */
        long file_size;
        recv_all(client, &file_size, sizeof(file_size));

        printf("Receiving: %s (%ld bytes)\n",
               filename, file_size);

        /* Receive C file */
        FILE *fp = fopen("source.c", "wb");

        long received = 0;

        while (received < file_size)
        {
            int n = (file_size - received > BUF)
                    ? BUF
                    : file_size - received;

            recv_all(client, buffer, n);
            fwrite(buffer, 1, n, fp);

            received += n;
        }

        fclose(fp);

        printf("File received.\n");

        /* Compile */
        printf("Compiling...\n");

        int status = system(
            "gcc source.c -o program 2> compile_error.txt"
        );

        /* Compilation failed */
        if (status != 0)
        {
            char error[BUF] = {0};

            FILE *err = fopen("compile_error.txt", "r");

            if (err)
            {
                fread(error, 1, BUF - 1, err);
                fclose(err);
            }

            int type = 0;   /* compilation error */
            int len = strlen(error);

            send_all(client, &type, sizeof(type));
            send_all(client, &len, sizeof(len));
            send_all(client, error, len);

            printf("Compilation failed.\n");
        }

        /* Compilation successful */
        else
        {
            int type = 1;   /* success */

            long exe_size;

            fp = fopen("program", "rb");

            fseek(fp, 0, SEEK_END);
            exe_size = ftell(fp);
            rewind(fp);

            /* Send status */
            send_all(client, &type, sizeof(type));

            /* Send executable size */
            send_all(client, &exe_size, sizeof(exe_size));

            printf("Compilation successful.\n");
            printf("Sending executable (%ld bytes)...\n",
                   exe_size);

            long sent = 0;

            while (sent < exe_size)
            {
                int n = fread(buffer, 1, BUF, fp);

                send_all(client, buffer, n);

                sent += n;
            }

            fclose(fp);

            printf("Executable sent.\n");
        }

        /* Cleanup */
        remove("source.c");
        remove("program");
        remove("compile_error.txt");

        close(client);

        printf("Client disconnected.\n");
    }

    close(server);

    return 0;
}
