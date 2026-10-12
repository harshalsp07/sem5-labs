#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct {
    int job_id;
    char operation[32];
    long long input_value;
} JobRequest;

typedef struct {
    int job_id;
    long long result;
    int status; // 0 = ok, -1 = error
    char message[64];
} JobResponse;

void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

long long compute_factorial(long long n, int *status, char *msg, size_t msg_len) {
    if (n < 0) {
        *status = -1;
        snprintf(msg, msg_len, "Factorial undefined for negative numbers");
        return 0;
    }
    if (n > 20) {
        *status = -1;
        snprintf(msg, msg_len, "Value too large (> 20)");
        return 0;
    }
    long long fact = 1;
    for (long long i = 2; i <= n; i++) {
        fact *= i;
    }
    *status = 0;
    snprintf(msg, msg_len, "OK");
    return fact;
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
        /* PARENT (JOB PRODUCER) */
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        printf("--- Job Processing System ---\n");

        char line[128];
        while (1) {
            printf("\nEnter Job ID (-1 to exit): ");
            if (!fgets(line, sizeof(line), stdin)) break;
            trim_newline(line);

            if (strlen(line) == 0) continue;
            int job_id = atoi(line);

            if (job_id == -1) {
                JobRequest term_job;
                term_job.job_id = -1;
                strcpy(term_job.operation, "exit");
                term_job.input_value = 0;
                write(parent_to_child[1], &term_job, sizeof(JobRequest));
                break;
            }

            printf("Enter Operation (square, cube, double, factorial): ");
            char op_input[64];
            if (!fgets(op_input, sizeof(op_input), stdin)) break;
            trim_newline(op_input);

            char operation[32];
            if (strcmp(op_input, "1") == 0 || strcasecmp(op_input, "square") == 0) {
                strcpy(operation, "square");
            } else if (strcmp(op_input, "2") == 0 || strcasecmp(op_input, "cube") == 0) {
                strcpy(operation, "cube");
            } else if (strcmp(op_input, "3") == 0 || strcasecmp(op_input, "double") == 0) {
                strcpy(operation, "double");
            } else if (strcmp(op_input, "4") == 0 || strcasecmp(op_input, "factorial") == 0) {
                strcpy(operation, "factorial");
            } else {
                snprintf(operation, sizeof(operation), "%s", op_input);
            }

            printf("Enter Input value: ");
            if (!fgets(line, sizeof(line), stdin)) break;
            long long val = atoll(line);

            JobRequest req;
            req.job_id = job_id;
            strncpy(req.operation, operation, sizeof(req.operation) - 1);
            req.operation[sizeof(req.operation) - 1] = '\0';
            req.input_value = val;

            write(parent_to_child[1], &req, sizeof(JobRequest));

            JobResponse resp;
            read(child_to_parent[0], &resp, sizeof(JobResponse));

            if (resp.status == 0) {
                printf("Result: %s(%lld) = %lld (Job ID: %d)\n",
                       req.operation, req.input_value, resp.result, resp.job_id);
            } else {
                printf("Error: %s (Job ID: %d)\n", resp.message, resp.job_id);
            }
        }

        close(parent_to_child[1]);
        close(child_to_parent[0]);

        waitpid(pid, NULL, 0);
        printf("Job processing complete. Exiting.\n");

    } else {
        /* CHILD (JOB PROCESSOR) */
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        JobRequest req;
        while (1) {
            ssize_t n = read(parent_to_child[0], &req, sizeof(JobRequest));
            if (n <= 0 || req.job_id == -1) {
                break;
            }

            JobResponse resp;
            resp.job_id = req.job_id;
            resp.status = 0;
            resp.result = 0;
            strcpy(resp.message, "OK");

            if (strcasecmp(req.operation, "square") == 0) {
                resp.result = req.input_value * req.input_value;
            } else if (strcasecmp(req.operation, "cube") == 0) {
                resp.result = req.input_value * req.input_value * req.input_value;
            } else if (strcasecmp(req.operation, "double") == 0) {
                resp.result = 2 * req.input_value;
            } else if (strcasecmp(req.operation, "factorial") == 0) {
                resp.result = compute_factorial(req.input_value, &resp.status,
                                                resp.message, sizeof(resp.message));
            } else {
                resp.status = -1;
                snprintf(resp.message, sizeof(resp.message),
                         "Unsupported operation '%s'", req.operation);
            }

            write(child_to_parent[1], &resp, sizeof(JobResponse));
        }

        close(parent_to_child[0]);
        close(child_to_parent[1]);
        exit(0);
    }

    return 0;
}
