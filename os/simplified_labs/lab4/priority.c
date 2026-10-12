/*
 * Lab 4: Priority CPU Scheduling (Non-Preemptive)
 *
 * Algorithm Concept:
 *   Among all arrived processes, the CPU selects the process with the highest
 *   priority (in this standard implementation: lowest priority number = highest priority).
 *
 * Compilation:
 *   gcc priority.c -o priority
 *
 * Usage:
 *   ./priority
 */

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int id;
    int at;        // Arrival Time
    int bt;        // Burst Time
    int priority;  // Priority value (1 is highest priority)
    int ct;        // Completion Time
    int tat;       // Turnaround Time
    int wt;        // Waiting Time
    bool done;
} Process;

int main() {
    // Sample processes: {ID, Arrival Time, Burst Time, Priority, ...}
    Process p[] = {
        {1, 0, 4, 3, 0, 0, 0, false},
        {2, 1, 3, 1, 0, 0, 0, false},
        {3, 2, 1, 4, 0, 0, 0, false},
        {4, 3, 5, 2, 0, 0, 0, false}
    };
    int n = sizeof(p) / sizeof(p[0]);

    int completed = 0;
    int current_time = 0;
    float total_tat = 0, total_wt = 0;

    while (completed < n) {
        int best_idx = -1;
        int highest_prio = 999999;

        // Find arrived process with highest priority (smallest priority number)
        for (int i = 0; i < n; i++) {
            if (!p[i].done && p[i].at <= current_time) {
                if (p[i].priority < highest_prio) {
                    highest_prio = p[i].priority;
                    best_idx = i;
                }
            }
        }

        // Idle time if no process has arrived
        if (best_idx == -1) {
            current_time++;
            continue;
        }

        // Run process to completion
        current_time += p[best_idx].bt;
        p[best_idx].ct = current_time;
        p[best_idx].tat = p[best_idx].ct - p[best_idx].at;
        p[best_idx].wt = p[best_idx].tat - p[best_idx].bt;
        p[best_idx].done = true;

        total_tat += p[best_idx].tat;
        total_wt += p[best_idx].wt;
        completed++;
    }

    // Print results table
    printf("=========================================================================\n");
    printf(" Priority CPU Scheduling (Lowest number = Highest Priority)\n");
    printf("=========================================================================\n");
    printf(" PID | Priority | Arrival Time | Burst Time | Completion | Turnaround | Waiting \n");
    printf("-------------------------------------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf(" P%-2d | %8d | %12d | %10d | %10d | %10d | %7d\n",
               p[i].id, p[i].priority, p[i].at, p[i].bt, p[i].ct, p[i].tat, p[i].wt);
    }
    printf("-------------------------------------------------------------------------\n");
    printf(" Average Turnaround Time = %.2f\n", total_tat / n);
    printf(" Average Waiting Time    = %.2f\n", total_wt / n);
    printf("=========================================================================\n");

    return 0;
}
