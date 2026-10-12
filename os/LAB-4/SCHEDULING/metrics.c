#include <stdio.h>
#include "metrics.h"

/*
 * Calculate all process-level scheduling metrics.
 */
void calculate_metrics(Simulation *sim)
{
    for (int i = 0; i < sim->process_count; i++)
    {
        Process *p = &sim->processes[i];

        /*
         * Turnaround Time
         */
        p->turnaround_time =
            p->completion_time - p->arrival_time;

        /*
         * Waiting Time
         */
        p->waiting_time =
            p->turnaround_time - p->burst_time;

        /*
         * Response Time
         */
        if (p->start_time >= 0)
        {
            p->response_time =
                p->start_time - p->arrival_time;
        }
        else
        {
            p->response_time = 0;
        }
    }
}


/*
 * Print individual process results.
 */
void print_results(Simulation *sim)
{
    double total_waiting = 0;
    double total_turnaround = 0;
    double total_response = 0;

    printf("\n");
    printf("============================================================\n");
    printf("                 SCHEDULING RESULTS\n");
    printf("============================================================\n");

    printf("\n");
    printf("%-6s %-6s %-6s %-6s %-6s %-6s %-6s\n",
           "PID",
           "AT",
           "BT",
           "CT",
           "TAT",
           "WT",
           "RT");

    printf("------------------------------------------------------------\n");

    for (int i = 0; i < sim->process_count; i++)
    {
        Process *p = &sim->processes[i];

        printf("%-6d %-6d %-6d %-6d %-6d %-6d %-6d\n",
               p->pid,
               p->arrival_time,
               p->burst_time,
               p->completion_time,
               p->turnaround_time,
               p->waiting_time,
               p->response_time);

        total_waiting += p->waiting_time;
        total_turnaround += p->turnaround_time;
        total_response += p->response_time;
    }

    printf("------------------------------------------------------------\n");

    printf("\nAverage Waiting Time    : %.2f\n",
           total_waiting / sim->process_count);

    printf("Average Turnaround Time : %.2f\n",
           total_turnaround / sim->process_count);

    printf("Average Response Time   : %.2f\n",
           total_response / sim->process_count);

    printf("Context Switches        : %d\n",
           sim->context_switches);

    printf("\n");
}


/*
 * Print a simple Gantt chart.
 *
 * This assumes that the simulator stores the execution
 * history in the future. If your current Simulation
 * structure does not have a Gantt history yet, use the
 * function below as a placeholder.
 */
void print_gantt_chart(Simulation *sim)
{
    printf("\n");
    printf("============================================================\n");
    printf("                    GANTT CHART\n");
    printf("============================================================\n\n");

    if (sim->history_count == 0)
    {
        printf("No execution history.\n");
        return;
    }


    /*
     * Print process blocks.
     */
    printf("|");

    int current_pid =
        sim->history[0].pid;

    for (int i = 1;
         i < sim->history_count;
         i++)
    {
        if (sim->history[i].pid != current_pid)
        {
            printf(" P%d |", current_pid);

            current_pid =
                sim->history[i].pid;
        }
    }

    printf(" P%d |\n", current_pid);


    /*
     * Print time boundaries.
     */
    printf("%-4d",
           sim->history[0].start_time);

    current_pid =
        sim->history[0].pid;

    for (int i = 1;
         i < sim->history_count;
         i++)
    {
        if (sim->history[i].pid != current_pid)
        {
            printf("%-5d",
                   sim->history[i].start_time);

            current_pid =
                sim->history[i].pid;
        }
    }

    printf("%d\n",
           sim->history[
               sim->history_count - 1
           ].end_time);
}
