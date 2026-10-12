#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    char buffer[1024];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    printf("Socket created successfully.\n");

    // Allow reuse of port
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    printf("Bind successful.\n");

    // Listen
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        // Accept client
        client_fd = accept(
            server_fd,
            (struct sockaddr *)&client_addr,
            &addr_len
        );

        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("\nClient connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // Communication loop
        while (1)
        {
            memset(buffer, 0, sizeof(buffer));

            int bytes = read(client_fd,
                             buffer,
                             sizeof(buffer) - 1);

            if (bytes <= 0)
            {
                printf("Client disconnected.\n");
                break;
            }

            buffer[bytes] = '\0';

            printf("Received: %s", buffer);

            // Check for EXIT
            if (strncmp(buffer, "EXIT", 4) == 0)
            {
                printf("Client requested exit.\n");
                break;
            }

            // Receive:
            // operation number1 number2
            //
            // Example:
            // ADD 10 20
            // SUB 50 20
            // MUL 5 6

            char operation[10];
            double num1, num2, result;

            if (sscanf(buffer, "%s %lf %lf",
                       operation,
                       &num1,
                       &num2) != 3)
            {
                char *error = "Invalid request\n";

                send(client_fd,
                     error,
                     strlen(error),
                     0);

                continue;
            }

            // Perform operation
            if (strcmp(operation, "ADD") == 0)
            {
                result = num1 + num2;
            }
            else if (strcmp(operation, "SUB") == 0)
            {
                result = num1 - num2;
            }
            else if (strcmp(operation, "MUL") == 0)
            {
                result = num1 * num2;
            }
            else
            {
                char *error = "Invalid operation\n";

                send(client_fd,
                     error,
                     strlen(error),
                     0);

                continue;
            }

            // Prepare response
            char response[1024];

            snprintf(response,
                     sizeof(response),
                     "Result = %.2f\n",
                     result);

            // Send result
            send(client_fd,
                 response,
                 strlen(response),
                 0);

            printf("Sent: %s", response);
        }

        close(client_fd);

        printf("Client connection closed.\n");
    }

    close(server_fd);

    return 0;
}
