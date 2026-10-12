#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pid_t pid;

    char message[100];
    char buffer[100];

    /*
     * Create pipe
     *
     * fd[0] = read end
     * fd[1] = write end
     */
    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(1);
    }

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }


    if (pid > 0)
    {
        /* =========================
           PARENT
           ========================= */

        close(fd[0]);

        printf("Parent: Enter message: ");

        fgets(message,
              sizeof(message),
              stdin);

        /*
         * Send message through pipe.
         */
        write(fd[1],
              message,
              strlen(message) + 1);

        close(fd[1]);

        wait(NULL);
    }
    else
    {
        /* =========================
           CHILD
           ========================= */

        close(fd[1]);

        /*
         * Read from pipe.
         */
        read(fd[0],
             buffer,
             sizeof(buffer));

        printf("\nChild received: %s",
               buffer);

        close(fd[0]);
    }

    return 0;
}
