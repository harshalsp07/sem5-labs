#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(STDERR_FILENO, "Usage: ./mycp <source> <destination>\n", 37);
        return EXIT_FAILURE;
    }

    int src_fd = open(argv[1], O_RDONLY);
    if (src_fd == -1) {
        perror("Error opening source file");
        return EXIT_FAILURE;
    }

    int dst_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd == -1) {
        perror("Error opening/creating destination file");
        close(src_fd);
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        ssize_t total_written = 0;
        while (total_written < bytes_read) {
            ssize_t written = write(dst_fd, buffer + total_written, bytes_read - total_written);
            if (written == -1) {
                perror("Error writing to destination file");
                close(src_fd);
                close(dst_fd);
                return EXIT_FAILURE;
            }
            total_written += written;
        }
    }

    if (bytes_read == -1) {
        perror("Error reading source file");
        close(src_fd);
        close(dst_fd);
        return EXIT_FAILURE;
    }

    close(src_fd);
    close(dst_fd);
    return EXIT_SUCCESS;
}