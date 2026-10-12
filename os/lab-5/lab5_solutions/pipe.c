#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    long long first;
    long long second;
} Numbers;

typedef struct {
    long long sum;
    long long difference;
    long long product;
} Results;

int main(void)
{
    int parent_to_child[2];
    int child_to_parent[2];
    pid_t pid;

    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid > 0) {
        Numbers numbers;
        Results results;

        close(parent_to_child[0]);
        close(child_to_parent[1]);

        printf("Enter two numbers: ");
        fflush(stdout);
        if (scanf("%lld %lld", &numbers.first, &numbers.second) != 2) {
            fprintf(stderr, "Invalid input.\n");
            close(parent_to_child[1]);
            close(child_to_parent[0]);
            waitpid(pid, NULL, 0);
            return 1;
        }

        if (write(parent_to_child[1], &numbers, sizeof(numbers)) != sizeof(numbers)) {
            perror("write");
            close(parent_to_child[1]);
            close(child_to_parent[0]);
            waitpid(pid, NULL, 0);
            return 1;
        }
        close(parent_to_child[1]);

        if (read(child_to_parent[0], &results, sizeof(results)) != sizeof(results)) {
            perror("read");
            close(child_to_parent[0]);
            waitpid(pid, NULL, 0);
            return 1;
        }
        close(child_to_parent[0]);

        printf("Addition: %lld\n", results.sum);
        printf("Subtraction: %lld\n", results.difference);
        printf("Multiplication: %lld\n", results.product);
        waitpid(pid, NULL, 0);
    } else {
        Numbers numbers;
        Results results;

        close(parent_to_child[1]);
        close(child_to_parent[0]);

        if (read(parent_to_child[0], &numbers, sizeof(numbers)) != sizeof(numbers)) {
            perror("read");
            close(parent_to_child[0]);
            close(child_to_parent[1]);
            return 1;
        }
        close(parent_to_child[0]);

        results.sum = numbers.first + numbers.second;
        results.difference = numbers.first - numbers.second;
        results.product = numbers.first * numbers.second;

        if (write(child_to_parent[1], &results, sizeof(results)) != sizeof(results)) {
            perror("write");
            close(child_to_parent[1]);
            return 1;
        }
        close(child_to_parent[1]);
    }

    return 0;
}
