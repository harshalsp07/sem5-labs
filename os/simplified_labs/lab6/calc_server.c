/*
 * Lab 6: Simple TCP Calculator Server
 *
 * Purpose:
 *   Listens for client connections on PORT 8080.
 *   Receives two numbers and an operator (e.g. "15 + 25").
 *   Computes the result and sends it back to the client.
 *
 * Compilation:
 *   gcc calc_server.c -o calc_server
 *
 * Usage:
 *   ./calc_server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addr_len = sizeof(address);
    char buffer[BUFFER_SIZE];

    // 1. Create TCP socket (IPv4, Stream/TCP)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Allow immediate reuse of the port
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Configure server address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Accept from any network interface
    address.sin_port = htons(PORT);

    // 3. Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    // 4. Listen for incoming connections (queue up to 5)
    listen(server_fd, 5);
    printf("=========================================\n");
    printf(" Calculator Server listening on port %d...\n", PORT);
    printf("=========================================\n");

    while (1) {
        // 5. Accept client connection
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected!\n");

        while (1) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

            if (bytes_read <= 0) {
                printf("Client disconnected.\n");
                break;
            }

            // Remove trailing newline if any
            buffer[strcspn(buffer, "\r\n")] = 0;

            // Exit check
            if (strcmp(buffer, "exit") == 0) {
                printf("Client requested exit.\n");
                break;
            }

            // Parse: num1 operator num2 (e.g., "10 + 5")
            double num1, num2, result = 0;
            char op;
            char response[BUFFER_SIZE];

            if (sscanf(buffer, "%lf %c %lf", &num1, &op, &num2) == 3) {
                int valid = 1;
                switch (op) {
                    case '+': result = num1 + num2; break;
                    case '-': result = num1 - num2; break;
                    case '*': result = num1 * num2; break;
                    case '/': 
                        if (num2 != 0) result = num1 / num2;
                        else valid = 0;
                        break;
                    default: valid = -1; break;
                }

                if (valid == 1) {
                    snprintf(response, sizeof(response), "Result: %.2lf\n", result);
                } else if (valid == 0) {
                    snprintf(response, sizeof(response), "Error: Division by zero!\n");
                } else {
                    snprintf(response, sizeof(response), "Error: Unknown operator '%c' (Use +, -, *, /)\n", op);
                }
            } else {
                snprintf(response, sizeof(response), "Error: Invalid format. Use: <num1> <op> <num2> (e.g. 10 + 5)\n");
            }

            send(client_fd, response, strlen(response), 0);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
