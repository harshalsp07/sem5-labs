#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/wait.h>

int main(){
    int fd[2]; 
    pid_t pid; 
    char write_msg[] = "Data sent through the pipe "; 
    char read_msg[100]; 

    pipe(fd); 
    pid = fork(); 

    if(pid < 0){
        perror("Fork failed"); 
        return 1; 
    }
    if(pid == 0){
        close(fd[0]); 
        write(fd[1],write_msg,strlen(write_msg)+1);
        close(fd[1]); 
        exit(0);  
    }
    else{
        close(fd[1]); 
        read(fd[0],read_msg,sizeof(read_msg)); 
        close(fd[0]); 
        wait(NULL); 
    }
    return 0 ; 

}