#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_FILENAME 255


/*
 * Send exactly 'length' bytes.
 */
int send_all(int sock, const void *buffer, size_t length)
{
    size_t total = 0;
    const char *ptr = (const char *)buffer;

    while (total < length)
    {
        ssize_t bytes = send(sock,
                             ptr + total,
                             length - total,
                             0);

        if (bytes <= 0)
        {
            return -1;
        }

        total += bytes;
    }

    return 0;
}


/*
 * Receive exactly 'length' bytes.
 */
int recv_all(int sock, void *buffer, size_t length)
{
    size_t total = 0;
    char *ptr = (char *)buffer;

    while (total < length)
    {
        ssize_t bytes = recv(sock,
                             ptr + total,
                             length - total,
                             0);

        if (bytes <= 0)
        {
            return -1;
        }

        total += bytes;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    int sock;

    struct sockaddr_in server_addr;

    char filename[MAX_FILENAME + 1];

    char buffer[BUFFER_SIZE];


    // ==================================================
    // 1. Check command-line argument
    // ==================================================

    if (argc != 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);

        return 1;
    }


    // ==================================================
    // 2. Create socket
    // ==================================================

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");

        return 1;
    }

    printf("Socket created successfully.\n");


    // ==================================================
    // 3. Configure server address
    // ==================================================

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;

    server_addr.sin_port = htons(PORT);


    // Convert IP address

    if (inet_pton(AF_INET,
                  argv[1],
                  &server_addr.sin_addr) <= 0)
    {
        perror("Invalid server IP address");

        close(sock);

        return 1;
    }


    // ==================================================
    // 4. Connect to server
    // ==================================================

    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");

        close(sock);

        return 1;
    }

    printf("Connected to server.\n");


    // ==================================================
    // 5. Ask for filename
    // ==================================================

    printf("\nEnter filename to send: ");

    scanf("%255s", filename);


    // ==================================================
    // 6. Open file
    // ==================================================

    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        perror("Failed to open file");

        close(sock);

        return 1;
    }


    // ==================================================
    // 7. Determine file size
    // ==================================================

    if (fseek(file, 0, SEEK_END) != 0)
    {
        perror("fseek failed");

        fclose(file);
        close(sock);

        return 1;
    }


    long long size = ftell(file);

    if (size < 0)
    {
        perror("ftell failed");

        fclose(file);
        close(sock);

        return 1;
    }


    uint64_t file_size = (uint64_t)size;


    // Return to beginning of file

    fseek(file, 0, SEEK_SET);


    printf("File size: %lu bytes\n",
           (unsigned long)file_size);


    // ==================================================
    // 8. Send filename length
    // ==================================================

    uint32_t filename_length =
        (uint32_t)strlen(filename);


    uint32_t network_filename_length =
        htonl(filename_length);


    if (send_all(sock,
                 &network_filename_length,
                 sizeof(network_filename_length)) < 0)
    {
        printf("Failed to send filename length.\n");

        fclose(file);
        close(sock);

        return 1;
    }


    // ==================================================
    // 9. Send filename
    // ==================================================

    if (send_all(sock,
                 filename,
                 filename_length) < 0)
    {
        printf("Failed to send filename.\n");

        fclose(file);
        close(sock);

        return 1;
    }


    // ==================================================
    // 10. Send file size
    // ==================================================

    uint64_t network_file_size =
        htobe64(file_size);


    if (send_all(sock,
                 &network_file_size,
                 sizeof(network_file_size)) < 0)
    {
        printf("Failed to send file size.\n");

        fclose(file);
        close(sock);

        return 1;
    }


    printf("\nSending file...\n");


    // ==================================================
    // 11. Read and send file data
    // ==================================================

    uint64_t total_sent = 0;


    while (1)
    {
        size_t bytes_read =
            fread(buffer,
                  1,
                  BUFFER_SIZE,
                  file);


        if (bytes_read > 0)
        {
            if (send_all(sock,
                         buffer,
                         bytes_read) < 0)
            {
                printf("\nFile transfer failed.\n");

                fclose(file);
                close(sock);

                return 1;
            }


            total_sent += bytes_read;


            printf("\rSent: %lu / %lu bytes",
                   (unsigned long)total_sent,
                   (unsigned long)file_size);

            fflush(stdout);
        }


        // End of file

        if (bytes_read < BUFFER_SIZE)
        {
            if (feof(file))
            {
                break;
            }

            if (ferror(file))
            {
                printf("\nError reading file.\n");

                fclose(file);
                close(sock);

                return 1;
            }
        }
    }


    printf("\n\nFile sent successfully!\n");


    // ==================================================
    // 12. Close input file
    // ==================================================

    fclose(file);


    // ==================================================
    // 13. Receive confirmation from server
    // ==================================================

    memset(buffer, 0, sizeof(buffer));


    int bytes_received =
        recv(sock,
             buffer,
             sizeof(buffer) - 1,
             0);


    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';


        if (strcmp(buffer, "UPLOAD_SUCCESS") == 0)
        {
            printf("Server confirmed: File uploaded successfully.\n");
        }
        else
        {
            printf("Server response: %s\n",
                   buffer);
        }
    }
    else
    {
        printf("No confirmation received from server.\n");
    }


    // ==================================================
    // 14. Close socket
    // ==================================================

    close(sock);


    printf("Connection closed.\n");


    return 0;
}
