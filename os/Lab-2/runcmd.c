#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <command> <arg>\n", argv[0]);
        return 1;
    }
    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork failed!\n");
        return 1;
    } 
    else if (pid == 0) {
        execlp(argv[1], argv[1], argv[2], NULL);
        printf("Command execution failed!\n");
        exit(1);
    } 
    else {
        wait(NULL);
        printf("Child process completed successfully.\n");
    }
    return 0;
}