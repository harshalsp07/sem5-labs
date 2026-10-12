#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h> 

#define BUFFER_SIZE 64


int main(void){
    const char *filepath = "example.txt";
    const char *text = "this is the low-level file handling hsp , in c . \n";
    char buffer[BUFFER_SIZE];

    int fd = open(filepath, O_CREAT | O_RDWR | O_TRUNC , 0644);
    if(fd == -1){
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_written = write(fd,text,strlen(text));
    if (bytes_written == -1){
        perror("erro writing the file ");
        close(fd);
        exit(EXIT_FAILURE);
        
    }
    if (lseek(fd,0,SEEK_SET) == (off_t)-1){
        perror("error seeking the file offset");
        close(fd);
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("Error reading from file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_read] = '\0'; // Null-terminate for string printing
    write(STDOUT_FILENO, "Read from file: ", 16);
    write(STDOUT_FILENO, buffer, bytes_read);

    // 5. Close descriptor
    if (close(fd) == -1) {
        perror("Error closing file");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}