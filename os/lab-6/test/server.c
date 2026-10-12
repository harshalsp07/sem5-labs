#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 8080

int main(){
    int server_fd,client_fd; 
    struct sockaddr_in address; 
    int opt = 1; 
    char buffer[1024] = {0}; 
    char *server_reply = "Hello from TCP server! "; 
    if((server_fd = socket(AF_INET,SOCK_STREAM,0))< 0 ){
        perror("Socket Creation failed"); 
        exit(EXIT_FAILURE);
    }
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt , sizeof(opt)); 

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr  = INADDR_ANY; 
    address.sin_port = htons(PORT); 

    if(bind(server_fd,(struct  sockaddr *)&address,sizeof(address)) < 0 )
    {
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }

    if(listen(server_fd,3) < 0 ){
        perror("Listed failed"); 
        exit(EXIT_FAILURE); 
    }
    printf("Server listening on port %d...\n", PORT);

    if((client_fd = accept(server_fd,NULL,NULL) ) < 0 ){
        perror("accept failed ");
        exit(EXIT_FAILURE); 
    }
    printf("cliend connected successfull "); 

    read(client_fd,buffer,sizeof(buffer)); 
    printf("Server received: %s\n", buffer);
    send(client_fd, server_reply,strlen(server_reply),0); 

    printf("Server reply sent.\n");

    // 9. Close network descriptors
    close(client_fd);
    close(server_fd);
    return 0;
    
}