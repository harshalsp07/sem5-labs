#include <stdio.h>
#include <unistd.h>

int main() {
    // First fork: 1 process -> 2 processes
    fork();

    // Second fork: both existing processes execute this
    if (fork() == 0) {
        // Third fork: only the children from the second fork execute this
        fork();
    }

    // Every resulting process executes this
    printf("PID: %d, Parent PID: %d\n", getpid(), getppid());

    return 0;
}
