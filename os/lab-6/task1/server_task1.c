#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created successfully.\n");

    // Allow reuse of port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Bind successful on port %d.\n", PORT);

    // Listen
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        // Accept client
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("\nClient connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // Communication loop for persistent connection
        while (1)
        {
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

            if (bytes_read <= 0)
            {
                printf("Client disconnected.\n");
                break;
            }

            buffer[bytes_read] = '\0';

            // Strip trailing newlines or carriage returns for cleaner logging
            buffer[strcspn(buffer, "\r\n")] = 0;

            printf("Received request: \"%s\"\n", buffer);

            // Check for EXIT
            if (strcmp(buffer, "EXIT") == 0)
            {
                printf("Client requested EXIT. Closing connection.\n");
                char *ack = "Session terminated.\n";
                send(client_fd, ack, strlen(ack), 0);
                break;
            }

            char operation[32];
            double value = 0.0;
            char response[BUFFER_SIZE];

            // Parse request
            if (sscanf(buffer, "%31s %lf", operation, &value) != 2)
            {
                snprintf(response, sizeof(response), "Error: Invalid request format. Use: <OPERATION> <value>\n");
                send(client_fd, response, strlen(response), 0);
                continue;
            }

            // Perform conversion
            if (strcmp(operation, "CM_TO_M") == 0)
            {
                double result = value / 100.0;
                snprintf(response, sizeof(response), "%g cm = %.2f m\n", value, result);
            }
            else if (strcmp(operation, "M_TO_KM") == 0)
            {
                double result = value / 1000.0;
                snprintf(response, sizeof(response), "%g m = %.2f km\n", value, result);
            }
            else if (strcmp(operation, "C_TO_F") == 0)
            {
                double result = (value * 9.0 / 5.0) + 32.0;
                snprintf(response, sizeof(response), "%g C = %.2f F\n", value, result);
            }
            else if (strcmp(operation, "KG_TO_G") == 0)
            {
                double result = value * 1000.0;
                snprintf(response, sizeof(response), "%g kg = %.2f g\n", value, result);
            }
            else
            {
                snprintf(response, sizeof(response), "Error: Unsupported request '%s'.\n", operation);
            }

            // Send response back to client
            send(client_fd, response, strlen(response), 0);
            printf("Sent response: %s", response);
        }

        close(client_fd);
        printf("Client connection closed. Waiting for new clients...\n");
    }

    close(server_fd);
    return 0;
}
