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
 * Receive exactly 'length' bytes.
 * TCP recv() may return fewer bytes than requested,
 * so we keep receiving until all bytes arrive.
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


/*
 * Send exactly 'length' bytes.
 * send() may send fewer bytes than requested,
 * so we keep sending until everything is sent.
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


int main()
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t addr_len = sizeof(client_addr);

    char filename[MAX_FILENAME + 1];
    char output_filename[MAX_FILENAME + 20];

    char buffer[BUFFER_SIZE];

    uint32_t filename_length;
    uint64_t file_size;

    uint64_t total_received = 0;


    // --------------------------------------------------
    // 1. Create socket
    // --------------------------------------------------

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    printf("Socket created successfully.\n");


    // --------------------------------------------------
    // 2. Allow port reuse
    // --------------------------------------------------

    int opt = 1;

    if (setsockopt(server_fd,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &opt,
                   sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(server_fd);
        exit(1);
    }


    // --------------------------------------------------
    // 3. Configure server address
    // --------------------------------------------------

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = INADDR_ANY;

    server_addr.sin_port = htons(PORT);


    // --------------------------------------------------
    // 4. Bind
    // --------------------------------------------------

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    printf("Bind successful.\n");


    // --------------------------------------------------
    // 5. Listen
    // --------------------------------------------------

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);


    // --------------------------------------------------
    // 6. Accept client
    // --------------------------------------------------

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &addr_len);

    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        exit(1);
    }


    printf("\nClient connected!\n");

    printf("Client IP   : %s\n",
           inet_ntoa(client_addr.sin_addr));

    printf("Client Port : %d\n",
           ntohs(client_addr.sin_port));


    // ==================================================
    // 7. RECEIVE FILENAME LENGTH
    // ==================================================

    if (recv_all(client_fd,
                 &filename_length,
                 sizeof(filename_length)) < 0)
    {
        printf("Failed to receive filename length.\n");

        close(client_fd);
        close(server_fd);

        return 1;
    }

    // Convert network byte order to host byte order
    filename_length = ntohl(filename_length);


    // Validate filename length

    if (filename_length == 0 ||
        filename_length > MAX_FILENAME)
    {
        printf("Invalid filename length.\n");

        close(client_fd);
        close(server_fd);

        return 1;
    }


    // ==================================================
    // 8. RECEIVE FILENAME
    // ==================================================

    memset(filename, 0, sizeof(filename));

    if (recv_all(client_fd,
                 filename,
                 filename_length) < 0)
    {
        printf("Failed to receive filename.\n");

        close(client_fd);
        close(server_fd);

        return 1;
    }

    filename[filename_length] = '\0';


    printf("\nFile name received: %s\n", filename);


    // ==================================================
    // 9. RECEIVE FILE SIZE
    // ==================================================

    if (recv_all(client_fd,
                 &file_size,
                 sizeof(file_size)) < 0)
    {
        printf("Failed to receive file size.\n");

        close(client_fd);
        close(server_fd);

        return 1;
    }

    // Convert from network byte order
    file_size = be64toh(file_size);


    printf("File size: %lu bytes\n",
           (unsigned long)file_size);


    // ==================================================
    // 10. Create output filename
    // ==================================================

    snprintf(output_filename,
             sizeof(output_filename),
             "received_%s",
             filename);


    // ==================================================
    // 11. Open output file
    // ==================================================

    FILE *file = fopen(output_filename, "wb");

    if (file == NULL)
    {
        perror("Failed to create output file");

        close(client_fd);
        close(server_fd);

        return 1;
    }


    printf("\nReceiving file...\n");


    // ==================================================
    // 12. Receive file data
    // ==================================================

    while (total_received < file_size)
    {
        uint64_t remaining =
            file_size - total_received;

        size_t bytes_to_receive = BUFFER_SIZE;

        if (remaining < BUFFER_SIZE)
        {
            bytes_to_receive = (size_t)remaining;
        }


        ssize_t bytes_received =
            recv(client_fd,
                 buffer,
                 bytes_to_receive,
                 0);


        if (bytes_received <= 0)
        {
            printf("\nConnection lost during file transfer.\n");

            fclose(file);
            close(client_fd);
            close(server_fd);

            return 1;
        }


        // Write received bytes to file

        size_t bytes_written =
            fwrite(buffer,
                   1,
                   bytes_received,
                   file);


        if (bytes_written != bytes_received)
        {
            printf("\nError writing file.\n");

            fclose(file);
            close(client_fd);
            close(server_fd);

            return 1;
        }


        total_received += bytes_received;


        // Display progress

        printf("\rReceived: %lu / %lu bytes",
               (unsigned long)total_received,
               (unsigned long)file_size);

        fflush(stdout);
    }


    // ==================================================
    // 13. Close output file
    // ==================================================

    fclose(file);


    printf("\n\nFile received successfully!\n");

    printf("Saved as: %s\n",
           output_filename);


    // ==================================================
    // 14. Send confirmation
    // ==================================================

    char response[] = "UPLOAD_SUCCESS";

    if (send_all(client_fd,
                 response,
                 strlen(response)) < 0)
    {
        printf("Failed to send confirmation.\n");
    }
    else
    {
        printf("Confirmation sent to client.\n");
    }


    // ==================================================
    // 15. Close connections
    // ==================================================

    close(client_fd);

    close(server_fd);

    printf("Connection closed.\n");


    return 0;
}
