/*
 * Lab 3: File Information Utility (fileinfo)
 *
 * Purpose:
 *   Counts total lines, words, and bytes in a given file,
 *   and checks if the file is readable and writable using access().
 *
 * Compilation:
 *   gcc fileinfo.c -o fileinfo
 *
 * Usage:
 *   ./fileinfo <filename>
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    long lines = 0;
    long words = 0;
    long bytes = 0;
    int in_word = 0;
    int ch;

    // Read character by character to accurately count lines, words, bytes
    while ((ch = fgetc(fp)) != EOF) {
        bytes++;

        if (ch == '\n') {
            lines++;
        }

        if (isspace((unsigned char)ch)) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            words++;
        }
    }

    fclose(fp);

    // Check file access permissions
    int is_readable = (access(filename, R_OK) == 0);
    int is_writable = (access(filename, W_OK) == 0);

    // Display formatted results
    printf("=========================================\n");
    printf(" File Information: %s\n", filename);
    printf("=========================================\n");
    printf(" Lines    : %ld\n", lines);
    printf(" Words    : %ld\n", words);
    printf(" Bytes    : %ld\n", bytes);
    printf(" Readable : %s\n", is_readable ? "Yes" : "No");
    printf(" Writable : %s\n", is_writable ? "Yes" : "No");
    printf("=========================================\n");

    return 0;
}
