#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    long total_bytes = 0;
    long total_lines = 0;
    long total_words = 0;
    int in_word = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        total_bytes += bytes_read;
        for (ssize_t i = 0; i < bytes_read; i++) {
            char ch = buffer[i];

            if (ch == '\n') {
                total_lines++;
            }

            if (isspace((unsigned char)ch)) {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                total_words++;
            }
        }
    }

    if (bytes_read == -1) {
        perror("Error reading file");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    // Determine read and write access permissions using POSIX access()
    int is_readable = (access(filename, R_OK) == 0);
    int is_writable = (access(filename, W_OK) == 0);

    printf("File: %s\n", filename);
    printf("Lines: %ld\n", total_lines);
    printf("Words: %ld\n", total_words);
    printf("Bytes: %ld\n", total_bytes);
    printf("Readable: %s\n", is_readable ? "Yes" : "No");
    printf("Writable: %s\n", is_writable ? "Yes" : "No");

    return EXIT_SUCCESS;
}