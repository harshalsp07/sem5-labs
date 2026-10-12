#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <arpa/inet.h>

#define PORT 8080 

int main(){
    int sock = 0; 
    struct sockaddr_in serv_addr; 
    char *client_msg = "Hello this is from client !";
    char buffer[1024] = {0}; 

    sock = socket(AF_INET,SOCK_STREAM,0); 
    
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 

    inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr); 

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }
    send(sock,client_msg,strlen(client_msg),0); 

    read(sock,buffer,sizeof(buffer)); 
    printf(buffer); 

    close(sock); 
    return 0 ; 
}