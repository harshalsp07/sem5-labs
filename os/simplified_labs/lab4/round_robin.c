/*
 * Lab 4: Round Robin (RR) CPU Scheduling
 *
 * Algorithm Concept:
 *   Each arrived ready process is given a fixed slice of CPU time (Time Quantum).
 *   If a process does not complete within this time quantum, it is preempted
 *   and placed at the end of the ready queue.
 *
 * Compilation:
 *   gcc round_robin.c -o round_robin
 *
 * Usage:
 *   ./round_robin
 */

#include <stdio.h>
#include <stdbool.h>

#define TIME_QUANTUM 2

typedef struct {
    int id;
    int at;        // Arrival Time
    int bt;        // Burst Time
    int rem_bt;    // Remaining Burst Time
    int ct;        // Completion Time
    int tat;       // Turnaround Time
    int wt;        // Waiting Time
} Process;

int main() {
    // Sample processes: {ID, Arrival Time, Burst Time, ...}
    Process p[] = {
        {1, 0, 5, 5, 0, 0, 0},
        {2, 1, 4, 4, 0, 0, 0},
        {3, 2, 2, 2, 0, 0, 0},
        {4, 4, 1, 1, 0, 0, 0}
    };
    int n = sizeof(p) / sizeof(p[0]);

    // Ready queue implemented as circular/simple array
    int queue[100];
    int front = 0, rear = 0;
    bool in_queue[100] = {false};

    int current_time = 0;
    int completed = 0;
    float total_tat = 0, total_wt = 0;

    // Add first arrived process
    queue[rear++] = 0;
    in_queue[0] = true;

    while (completed < n) {
        if (front == rear) {
            // Queue is empty, find next arriving process
            for (int i = 0; i < n; i++) {
                if (p[i].rem_bt > 0) {
                    current_time = p[i].at;
                    queue[rear++] = i;
                    in_queue[i] = true;
                    break;
                }
            }
        }

        int curr = queue[front++];
        int exec_time = (p[curr].rem_bt < TIME_QUANTUM) ? p[curr].rem_bt : TIME_QUANTUM;

        p[curr].rem_bt -= exec_time;
        current_time += exec_time;

        // Check if any new process arrived during this execution slice
        for (int i = 0; i < n; i++) {
            if (i != curr && !in_queue[i] && p[i].at <= current_time && p[i].rem_bt > 0) {
                queue[rear++] = i;
                in_queue[i] = true;
            }
        }

        // If current process is still not finished, re-enqueue it
        if (p[curr].rem_bt > 0) {
            queue[rear++] = curr;
        } else {
            // Process completed
            p[curr].ct = current_time;
            p[curr].tat = p[curr].ct - p[curr].at;
            p[curr].wt = p[curr].tat - p[curr].bt;

            total_tat += p[curr].tat;
            total_wt += p[curr].wt;
            completed++;
        }
    }

    // Print results table
    printf("=========================================================================\n");
    printf(" Round Robin CPU Scheduling (Time Quantum = %d)\n", TIME_QUANTUM);
    printf("=========================================================================\n");
    printf(" PID | Arrival Time | Burst Time | Completion | Turnaround | Waiting \n");
    printf("-------------------------------------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf(" P%-2d | %12d | %10d | %10d | %10d | %7d\n",
               p[i].id, p[i].at, p[i].bt, p[i].ct, p[i].tat, p[i].wt);
    }
    printf("-------------------------------------------------------------------------\n");
    printf(" Average Turnaround Time = %.2f\n", total_tat / n);
    printf(" Average Waiting Time    = %.2f\n", total_wt / n);
    printf("=========================================================================\n");

    return 0;
}
