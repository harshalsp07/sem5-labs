/*
 * Lab 4: First-Come First-Served (FCFS) CPU Scheduling
 *
 * Algorithm Concept:
 *   Processes are scheduled in order of their Arrival Time (AT).
 *   Formulas:
 *     Completion Time (CT) = max(current_time, AT) + BT
 *     Turnaround Time (TAT) = CT - AT
 *     Waiting Time (WT)     = TAT - BT
 *
 * Compilation:
 *   gcc fcfs.c -o fcfs
 *
 * Usage:
 *   ./fcfs
 */

#include <stdio.h>

typedef struct {
    int id;
    int at;   // Arrival Time
    int bt;   // Burst Time
    int ct;   // Completion Time
    int tat;  // Turnaround Time
    int wt;   // Waiting Time
} Process;

void sort_by_arrival(Process p[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (p[j].at > p[j + 1].at) {
                Process temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;
            }
        }
    }
}

int main() {
    // Sample processes: {ID, Arrival Time, Burst Time, CT, TAT, WT}
    Process p[] = {
        {1, 0, 4, 0, 0, 0},
        {2, 1, 3, 0, 0, 0},
        {3, 2, 1, 0, 0, 0},
        {4, 3, 2, 0, 0, 0},
        {5, 4, 5, 0, 0, 0}
    };
    int n = sizeof(p) / sizeof(p[0]);

    // Sort by arrival time
    sort_by_arrival(p, n);

    int current_time = 0;
    float total_tat = 0, total_wt = 0;

    for (int i = 0; i < n; i++) {
        // If CPU is idle, jump to the arrival time
        if (current_time < p[i].at) {
            current_time = p[i].at;
        }

        p[i].ct = current_time + p[i].bt;
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;

        current_time = p[i].ct;
        total_tat += p[i].tat;
        total_wt += p[i].wt;
    }

    // Print table
    printf("===============================================================\n");
    printf(" FCFS CPU Scheduling\n");
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
