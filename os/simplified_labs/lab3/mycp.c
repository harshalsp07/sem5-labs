/*
 * Lab 3: File Copy Utility (mycp)
 *
 * Purpose:
 *   Copies contents from <source> to <destination> using low-level
 *   system calls: open(), read(), write(), and close().
 *
 * Compilation:
 *   gcc mycp.c -o mycp
 *
 * Usage:
 *   ./mycp source.txt destination.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <source_file> <destination_file>\n", argv[0]);
        return 1;
    }

    // 1. Open source file in Read-Only mode
    int src_fd = open(argv[1], O_RDONLY);
    if (src_fd < 0) {
        perror("Error opening source file");
        return 1;
    }

    // 2. Open/create destination file in Write-Only mode (overwrite if exists)
    int dst_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd < 0) {
        perror("Error opening destination file");
        close(src_fd);
        return 1;
    }

    // 3. Read from source and write to destination
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dst_fd, buffer, bytes_read);
        if (bytes_written < 0) {
            perror("Error writing to destination file");
            close(src_fd);
            close(dst_fd);
            return 1;
        }
    }

    // 4. Close both file descriptors
    close(src_fd);
    close(dst_fd);

    printf("File copied successfully from '%s' to '%s'.\n", argv[1], argv[2]);
    return 0;
}
