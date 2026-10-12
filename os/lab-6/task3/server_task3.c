#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

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

int main()
{
    int server, client;
    struct sockaddr_in addr;
    char filename[256];
    char buffer[BUF];

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind failed");
        close(server);
        exit(1);
    }

    if (listen(server, 5) < 0)
    {
        perror("Listen failed");
        close(server);
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        client = accept(server, (struct sockaddr *)&client_addr, &client_len);
        if (client < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("\nClient connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        /* Receive filename */
        int name_len;
        if (recv_all(client, &name_len, sizeof(name_len)) < 0 || name_len <= 0 || name_len >= 256)
        {
            printf("Failed to receive valid filename length.\n");
            close(client);
            continue;
        }

        if (recv_all(client, filename, name_len) < 0)
        {
            printf("Failed to receive filename.\n");
            close(client);
            continue;
        }
        filename[name_len] = '\0';

        /* Receive file size */
        long file_size;
        if (recv_all(client, &file_size, sizeof(file_size)) < 0 || file_size < 0)
        {
            printf("Failed to receive file size.\n");
            close(client);
            continue;
        }

        printf("Receiving source file: %s (%ld bytes)\n", filename, file_size);

        /* Save C source file */
        FILE *fp = fopen("received_source.c", "wb");
        if (!fp)
        {
            perror("Failed to create received_source.c");
            close(client);
            continue;
        }

        long received = 0;
        while (received < file_size)
        {
            int n = (file_size - received > BUF) ? BUF : (int)(file_size - received);
            if (recv_all(client, buffer, n) < 0)
            {
                printf("Error receiving file data.\n");
                break;
            }
            fwrite(buffer, 1, n, fp);
            received += n;
        }
        fclose(fp);

        printf("Source file received successfully.\n");

        /* Compile with gcc */
        printf("Compiling source file...\n");
        int status = system("gcc received_source.c -o executed_program 2> compile_error.txt");

        /* If compilation failed */
        if (status != 0)
        {
            printf("Compilation failed. Sending compiler errors to client.\n");
            char error[BUF * 4] = {0};
            FILE *err_fp = fopen("compile_error.txt", "r");
            if (err_fp)
            {
                size_t r = fread(error, 1, sizeof(error) - 1, err_fp);
                error[r] = '\0';
                fclose(err_fp);
            }

            int type = 0; /* 0 = compilation error */
            int len = strlen(error);

            send_all(client, &type, sizeof(type));
            send_all(client, &len, sizeof(len));
            send_all(client, error, len);
        }
        else
        {
            printf("Compilation successful. Executing program using fork(), exec(), and pipe()...\n");

            int pipefd[2];
            if (pipe(pipefd) == -1)
            {
                perror("Pipe creation failed");
                int type = 0;
                char *msg = "Server internal error: pipe() failed\n";
                int len = strlen(msg);
                send_all(client, &type, sizeof(type));
                send_all(client, &len, sizeof(len));
                send_all(client, msg, len);
            }
            else
            {
                pid_t pid = fork();
                if (pid < 0)
                {
                    perror("Fork failed");
                    close(pipefd[0]);
                    close(pipefd[1]);
                    int type = 0;
                    char *msg = "Server internal error: fork() failed\n";
                    int len = strlen(msg);
                    send_all(client, &type, sizeof(type));
                    send_all(client, &len, sizeof(len));
                    send_all(client, msg, len);
                }
                else if (pid == 0)
                {
                    /* Child process: execute program and redirect stdout & stderr to pipe */
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    dup2(pipefd[1], STDERR_FILENO);
                    close(pipefd[1]);

                    execl("./executed_program", "./executed_program", (char *)NULL);
                    perror("execl failed");
                    _exit(127);
                }
                else
                {
                    /* Parent process: close write end, read output from read end */
                    close(pipefd[1]);

                    char exec_output[BUF * 8];
                    int total_out = 0;
                    int r;

                    while ((r = read(pipefd[0], exec_output + total_out, sizeof(exec_output) - 1 - total_out)) > 0)
                    {
                        total_out += r;
                    }
                    exec_output[total_out] = '\0';
                    close(pipefd[0]);

                    int child_status;
                    waitpid(pid, &child_status, 0);

                    printf("Execution finished. Output size: %d bytes.\n", total_out);
                    printf("Sending execution output to client directly...\n");

                    int type = 1; /* 1 = execution output */
                    send_all(client, &type, sizeof(type));
                    send_all(client, &total_out, sizeof(total_out));
                    send_all(client, exec_output, total_out);
                }
            }
        }

        /* Clean up temporary files */
        remove("received_source.c");
        remove("executed_program");
        remove("compile_error.txt");

        close(client);
        printf("Client connection closed. Waiting for new requests...\n");
    }

    close(server);
    return 0;
}
