#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    char line[1024];
    char *pattern;
    int found = 0;
    int line_number = 0;

    if (argc != 3) {
        printf("Usage: %s <pattern> <file>\n", argv[0]);
        return 1;
    }

    pattern = argv[1];
    fp = fopen(argv[2], "r");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_number++;
        int i = 0;
        while (line[i] != '\0') {
            int j = 0;
            if (line[i] == pattern[j]) {
                int k = i;
                while (pattern[j] != '\0' && line[k] == pattern[j]) {
                    k++;
                    j++;
                }
                if (pattern[j] == '\0') {
                    printf("Line %d: %s", line_number, line);
                    found = 1;
                    break;
                }
            }
            i++;
        }
    }

    fclose(fp);

    if (!found) {
        return 1;
    }

    return 0;
}