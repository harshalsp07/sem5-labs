/*
 * Lab 4: Shortest Remaining Time First (SRTF) Scheduling (Preemptive)
 *
 * Algorithm Concept:
 *   At each time tick (1 unit), the scheduler picks the arrived process
 *   with the smallest remaining burst time. If a new process arrives with
 *   a shorter burst time, the current process is preempted.
 *
 * Compilation:
 *   gcc srtf.c -o srtf
 *
 * Usage:
 *   ./srtf
 */

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int id;
    int at;        // Arrival Time
    int bt;        // Total Burst Time
    int rem_bt;    // Remaining Burst Time
    int ct;        // Completion Time
    int tat;       // Turnaround Time
    int wt;        // Waiting Time
} Process;

int main() {
    // Sample processes: {ID, Arrival Time, Burst Time, Remaining Time, ...}
    Process p[] = {
        {1, 0, 8, 8, 0, 0, 0},
        {2, 1, 4, 4, 0, 0, 0},
        {3, 2, 2, 2, 0, 0, 0},
        {4, 3, 1, 1, 0, 0, 0},
        {5, 4, 3, 2, 0, 0, 0}
    };
    p[4].rem_bt = 3; // Ensure remaining matches total BT
    int n = sizeof(p) / sizeof(p[0]);

    int completed = 0;
    int current_time = 0;
    float total_tat = 0, total_wt = 0;

    while (completed < n) {
        int shortest_idx = -1;
        int min_rem = 999999;

        // Find arrived process with shortest remaining burst time
        for (int i = 0; i < n; i++) {
            if (p[i].at <= current_time && p[i].rem_bt > 0) {
                if (p[i].rem_bt < min_rem) {
                    min_rem = p[i].rem_bt;
                    shortest_idx = i;
                }
            }
        }

        // CPU idle if no process has arrived
        if (shortest_idx == -1) {
            current_time++;
            continue;
        }

        // Execute for 1 time unit
        p[shortest_idx].rem_bt--;
        current_time++;

        // If process finished execution
        if (p[shortest_idx].rem_bt == 0) {
            p[shortest_idx].ct = current_time;
            p[shortest_idx].tat = p[shortest_idx].ct - p[shortest_idx].at;
            p[shortest_idx].wt = p[shortest_idx].tat - p[shortest_idx].bt;

            total_tat += p[shortest_idx].tat;
            total_wt += p[shortest_idx].wt;
            completed++;
        }
    }

    // Print results table
    printf("===============================================================\n");
    printf(" SRTF (Preemptive SJF) CPU Scheduling\n");
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
