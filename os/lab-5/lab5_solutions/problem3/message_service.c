#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 512

void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int main(void) {
    int parent_to_child[2];
    int child_to_parent[2];
    pid_t pid;

    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid > 0) {
        /* PARENT PROCESS */
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        char send_buf[BUFFER_SIZE];
        char recv_buf[BUFFER_SIZE];

        printf("--- Parent-Child Message Service ---\n");

        while (1) {
            printf("\nParent: Enter message (or 'exit' to quit): ");
            if (!fgets(send_buf, sizeof(send_buf), stdin)) {
                break;
            }
            trim_newline(send_buf);

            if (strlen(send_buf) == 0) {
                continue;
            }

            write(parent_to_child[1], send_buf, strlen(send_buf) + 1);

            if (strcasecmp(send_buf, "exit") == 0) {
                break;
            }

            ssize_t n = read(child_to_parent[0], recv_buf, sizeof(recv_buf) - 1);
            if (n > 0) {
                recv_buf[n] = '\0';
                printf("Parent received ACK: %s\n", recv_buf);
            } else {
                break;
            }
        }

        close(parent_to_child[1]);
        close(child_to_parent[0]);

        waitpid(pid, NULL, 0);
        printf("Parent: Child finished. Exiting.\n");

    } else {
        /* CHILD PROCESS */
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        char recv_buf[BUFFER_SIZE];
        char ack_buf[BUFFER_SIZE * 2];

        while (1) {
            ssize_t n = read(parent_to_child[0], recv_buf, sizeof(recv_buf) - 1);
            if (n <= 0) {
                break;
            }
            recv_buf[n] = '\0';

            if (strcasecmp(recv_buf, "exit") == 0) {
                break;
            }

            printf("Child received: %s\n", recv_buf);

            snprintf(ack_buf, sizeof(ack_buf), "Received '%s' successfully", recv_buf);
            write(child_to_parent[1], ack_buf, strlen(ack_buf) + 1);
        }

        close(parent_to_child[0]);
        close(child_to_parent[1]);
        exit(0);
    }

    return 0;
}
