/*
 * Lab 3: Pattern Search Utility (mygrep)
 *
 * Purpose:
 *   Searches for a word in a text file and prints each matching line
 *   along with its line number.
 *
 * Compilation:
 *   gcc mygrep.c -o mygrep
 *
 * Usage:
 *   ./mygrep <search_word> <filename>
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <search_word> <filename>\n", argv[0]);
        return 1;
    }

    const char *word = argv[1];
    const char *filename = argv[2];

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    char line[2048];
    int line_number = 1;
    int found_count = 0;

    // Read line-by-line and check if word exists in line
    while (fgets(line, sizeof(line), fp) != NULL) {
        // strcasestr performs case-insensitive substring search
        if (strcasestr(line, word) != NULL) {
            printf("%d: %s", line_number, line);
            found_count++;
        }
        line_number++;
    }

    fclose(fp);

    if (found_count == 0) {
        printf("No matches found for '%s' in '%s'.\n", word, filename);
    }

    return 0;
}
