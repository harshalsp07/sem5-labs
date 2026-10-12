/*
 * Lab 5 Solutions - Problem 4: Job Processing System (Pipes)
 *
 * Purpose:
 *   Demonstrates a Master-Worker job system using pipes:
 *     - Master (Parent) accepts computational tasks from user (e.g. factorial, square).
 *     - Worker (Child) performs calculations and returns the computed result.
 *
 * Compilation:
 *   gcc job_system.c -o job_system
 *
 * Usage:
 *   ./job_system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct {
    int job_id;
    char operation[16]; // "fact", "square", "cube"
    long long value;
} JobRequest;

typedef struct {
    int job_id;
    long long result;
    int success;        // 1 = success, 0 = error
    char msg[64];
} JobResponse;

int main() {
    int req_pipe[2]; // Master -> Worker
    int res_pipe[2]; // Worker -> Master

    if (pipe(req_pipe) == -1 || pipe(res_pipe) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid > 0) {
        // --- MASTER (Parent Process) ---
        close(req_pipe[0]); // Close read end of request pipe
        close(res_pipe[1]); // Close write end of response pipe

        printf("=========================================\n");
        printf(" Master-Worker Job System\n");
        printf(" Operations supported: fact, square, cube\n");
        printf(" Format: <operation> <number> (or 'exit')\n");
        printf(" Example: fact 5\n");
        printf(" Example: square 12\n");
        printf("=========================================\n");

        int next_id = 1;
        char input[128];

        while (1) {
            printf("\njob> ");
            fflush(stdout);

            if (fgets(input, sizeof(input), stdin) == NULL) break;
            input[strcspn(input, "\r\n")] = 0;

            if (strcmp(input, "exit") == 0) {
                JobRequest req = {-1, "exit", 0};
                write(req_pipe[1], &req, sizeof(req));
                printf("Shutting down worker...\n");
                break;
            }

            JobRequest req;
            req.job_id = next_id++;

            if (sscanf(input, "%15s %lld", req.operation, &req.value) != 2) {
                printf("Invalid format! Use: <operation> <number> (e.g. square 8)\n");
                continue;
            }

            // Send job request to worker
            write(req_pipe[1], &req, sizeof(req));

            // Read response from worker
            JobResponse res;
            read(res_pipe[0], &res, sizeof(res));

            if (res.success) {
                printf("[Job #%d Completed] Result = %lld\n", res.job_id, res.result);
            } else {
                printf("[Job #%d Failed] Error: %s\n", res.job_id, res.msg);
            }
        }

        close(req_pipe[1]);
        close(res_pipe[0]);
        wait(NULL);
    } else {
        // --- WORKER (Child Process) ---
        close(req_pipe[1]); // Close write end of request pipe
        close(res_pipe[0]); // Close read end of response pipe

        JobRequest req;
        while (read(req_pipe[0], &req, sizeof(req)) > 0) {
            if (strcmp(req.operation, "exit") == 0) {
                break;
            }

            JobResponse res;
            res.job_id = req.job_id;
            res.success = 1;
            strcpy(res.msg, "OK");

            if (strcmp(req.operation, "fact") == 0) {
                if (req.value < 0 || req.value > 20) {
                    res.success = 0;
                    strcpy(res.msg, "Number must be between 0 and 20");
                } else {
                    long long f = 1;
                    for (int i = 2; i <= req.value; i++) f *= i;
                    res.result = f;
                }
            } else if (strcmp(req.operation, "square") == 0) {
                res.result = req.value * req.value;
            } else if (strcmp(req.operation, "cube") == 0) {
                res.result = req.value * req.value * req.value;
            } else {
                res.success = 0;
                snprintf(res.msg, sizeof(res.msg), "Unknown operation '%s'", req.operation);
            }

            // Send response back to master
            write(res_pipe[1], &res, sizeof(res));
        }

        close(req_pipe[0]);
        close(res_pipe[1]);
        exit(0);
    }

    return 0;
}
