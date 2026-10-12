#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MAX_LINE_LEN 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <search_word> <filename>\n", argv[0]);
        return EXIT_FAILURE;


        
    }

    const char *search_word = argv[1];
    const char *filename = argv[2];

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    char line[MAX_LINE_LEN];
    size_t line_idx = 0;
    int line_number = 1;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            char ch = buffer[i];
            if (ch == '\n') {
                line[line_idx] = '\0';
                // strcasestr performs case-insensitive search matching the assignment example
                if (strcasestr(line, search_word) != NULL) {
                    printf("%d: %s\n", line_number, line);
                }
                line_number++;
                line_idx = 0;
            } else {
                if (line_idx < MAX_LINE_LEN - 1) {
                    line[line_idx++] = ch;
                }
            }
        }
    }

    // Check trailing line if file does not terminate with a newline character
    if (line_idx > 0) {
        line[line_idx] = '\0';
        if (strcasestr(line, search_word) != NULL) {
            printf("%d: %s\n", line_number, line);
        }
    }

    if (bytes_read == -1) {
        perror("Error reading file");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);
    return EXIT_SUCCESS;
}