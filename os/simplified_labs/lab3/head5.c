/*
 * Lab 3: First 5 Lines Printer (head5)
 *
 * Purpose:
 *   Prints the first 5 lines of a text file.
 *
 * Compilation:
 *   gcc head5.c -o head5
 *
 * Usage:
 *   ./head5 filename.txt
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    char line[1024];
    int line_count = 0;

    // Read and print line by line up to 5 lines
    while (line_count < 5 && fgets(line, sizeof(line), fp) != NULL) {
        printf("%s", line);
        line_count++;
    }

    fclose(fp);
    return 0;
}
