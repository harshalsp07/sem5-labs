#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define TARGET_LINES 5

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("enter the function with poper input \n");
        return EXIT_FAILURE;
    }

    int fd = open(argv[1],O_RDONLY);
    if (fd == -1){
        perror("Error opening file");
        return EXIT_FAILURE;

    }
   char buffer[BUFFER_SIZE];
   ssize_t bytes_read;
   int line_count = 0;
   
   while(line_count < TARGET_LINES && (bytes_read = read(fd,buffer,sizeof(buffer))) > 0 ){
        for(size_t i = 0 ; i < bytes_read ; i++ ){
            if(write(STDOUT_FILENO, &buffer[i],1)== -1 ){
               perror("error writing to stdout");
               close(fd);
               return EXIT_FAILURE; 
            }
            if( buffer[i]== '\n'){
                line_count++;
                if ( line_count == TARGET_LINES) break;
            }
        }       
    }

     if(bytes_read == -1){
        perror("error reading file");
        close(fd);
        return EXIT_FAILURE;
    }
    if(close(fd)== -1){
        perror("Error closign file");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}