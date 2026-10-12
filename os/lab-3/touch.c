#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main ( int argc , char *argv[]){
    int n = 1;

    if(argc < 2){
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    while(n < argc){
        int fd = open(argv[n], O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if(fd == -1){
            perror("open");
            return 1;
        }
        close(fd);
        n++;
    }
    return 0;
}