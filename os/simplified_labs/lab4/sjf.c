/*
 * Lab 4: Shortest Job First (SJF) Scheduling (Non-Preemptive)
 *
 * Algorithm Concept:
 *   Out of all processes that have arrived by current_time,
 *   the one with the smallest Burst Time (BT) is selected to run to completion.
 *
 * Compilation:
 *   gcc sjf.c -o sjf
 *
 * Usage:
 *   ./sjf
 */

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int id;
    int at;   // Arrival Time
    int bt;   // Burst Time
    int ct;   // Completion Time
    int tat;  // Turnaround Time
    int wt;   // Waiting Time
    bool done;
} Process;

int main() {
    // Sample processes: {ID, Arrival Time, Burst Time, 0, 0, 0, false}
    Process p[] = {
        {1, 1, 7, 0, 0, 0, false},
        {2, 2, 5, 0, 0, 0, false},
        {3, 3, 1, 0, 0, 0, false},
        {4, 4, 2, 0, 0, 0, false},
        {5, 5, 8, 0, 0, 0, false}
    };
    int n = sizeof(p) / sizeof(p[0]);

    int completed = 0;
    int current_time = 0;
    float total_tat = 0, total_wt = 0;

    while (completed < n) {
        int shortest_idx = -1;
        int min_bt = 999999;

        // Find arrived process with shortest burst time
        for (int i = 0; i < n; i++) {
            if (!p[i].done && p[i].at <= current_time) {
                if (p[i].bt < min_bt) {
                    min_bt = p[i].bt;
                    shortest_idx = i;
                }
            }
        }

        // If no process has arrived yet, advance clock
        if (shortest_idx == -1) {
            current_time++;
            continue;
        }

        // Execute selected process
        current_time += p[shortest_idx].bt;
        p[shortest_idx].ct = current_time;
        p[shortest_idx].tat = p[shortest_idx].ct - p[shortest_idx].at;
        p[shortest_idx].wt = p[shortest_idx].tat - p[shortest_idx].bt;
        p[shortest_idx].done = true;

        total_tat += p[shortest_idx].tat;
        total_wt += p[shortest_idx].wt;
        completed++;
    }

    // Print results table
    printf("===============================================================\n");
    printf(" SJF (Non-Preemptive) CPU Scheduling\n");
    printf("===============================================================\n");
    printf(" PID | Arrival Time | Burst Time | Completion | Turnaround | Waiting \n");
    printf("---------------------------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf(" P%-2d | %12d | %10d | %10d | %10d | %7d\n",
               p[i].id, p[i].at, p[i].bt, p[i].ct, p[i].tat, p[i].wt);
    }
    printf("---------------------------------------------------------------\n");
    printf(" Average Turnaround Time = %.2f\n", total_tat / n);
    printf(" Average Waiting Time    = %.2f\n", total_wt / n);
    printf("===============================================================\n");

    return 0;
}
