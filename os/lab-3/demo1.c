#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *filename = "student.txt";
    const char *message = "Name: Harshal Patel\nRoll No: B24CS016\nCourse: B.Tech. CSE 2028\nSubject: Operating System\n";
    char buffer[256];
    int fd, n;

    /* 1. Open the file for writing */
    fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    /* 2. Write the student information using write() */
    n = write(fd, message, strlen(message));
    if (n == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    /* 3. Close the file */
    close(fd);

    /* 4. Reopen it using open() with O_RDONLY */
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    /* 5. Read the contents using read() */
    n = read(fd, buffer, sizeof(buffer) - 1);
    if (n == -1) {
        perror("read");
        close(fd);
        return 1;
    }
    buffer[n] = '\0';

    /* 6. Display the contents on the screen */
    printf("Contents of %s:\n%s", filename, buffer);

    /* 7. Close the file */
    close(fd);

    return 0;
}