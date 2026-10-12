#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void print_help()
{
    printf("\nSupported Requests:\n");
    printf("  CM_TO_M <value>   - Convert Centimeters to Meters\n");
    printf("  M_TO_KM <value>   - Convert Meters to Kilometers\n");
    printf("  C_TO_F <value>    - Convert Celsius to Fahrenheit\n");
    printf("  KG_TO_G <value>   - Convert Kilograms to Grams\n");
    printf("  EXIT              - Disconnect and exit\n");
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

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

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("Invalid server IP address");
        close(sock);
        return 1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("=========================================\n");
    printf("    Connected to Unit Converter Server   \n");
    printf("=========================================\n");
    print_help();
    printf("=========================================\n\n");

    while (1)
    {
        printf("Enter request > ");
        if (!fgets(input, sizeof(input), stdin))
        {
            printf("\nExiting...\n");
            break;
        }

        // Remove trailing newline
        input[strcspn(input, "\r\n")] = 0;

        // Skip empty input
        if (strlen(input) == 0)
        {
            continue;
        }

        // Send request to server
        if (send(sock, input, strlen(input), 0) < 0)
        {
            perror("Failed to send request");
            break;
        }

        // If command is EXIT, break after sending
        if (strcmp(input, "EXIT") == 0)
        {
            printf("Exiting client session...\n");
            break;
        }

        // Receive response
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = read(sock, buffer, sizeof(buffer) - 1);
        if (bytes > 0)
        {
            buffer[bytes] = '\0';
            printf("Server response: %s", buffer);
        }
        else
        {
            printf("Server disconnected.\n");
            break;
        }
    }

    close(sock);
    return 0;
}
