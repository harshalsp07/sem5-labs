#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simulator.h"


/*
 * Scheduler functions implemented in algorithms/
 */
int fcfs_scheduler(Simulation *sim);
int lcfs_scheduler(Simulation *sim);
int ljf_scheduler(Simulation *sim);
int sjf_scheduler(Simulation *sim);
int srtf_scheduler(Simulation *sim);
int priority_scheduler(Simulation *sim);
int rr_scheduler(Simulation *sim);


/*
 * Return the scheduler corresponding to
 * the command-line algorithm name.
 */
SchedulerFunction get_scheduler(const char *name)
{
    if (strcmp(name, "ljf") == 0)
        return ljf_scheduler;

    if (strcmp(name, "lcfs") == 0)
        return lcfs_scheduler;

    if (strcmp(name, "fcfs") == 0)
        return fcfs_scheduler;

    if (strcmp(name, "sjf") == 0)
        return sjf_scheduler;

    if (strcmp(name, "srtf") == 0)
        return srtf_scheduler;

    if (strcmp(name, "priority") == 0)
        return priority_scheduler;

    if (strcmp(name, "rr") == 0)
        return rr_scheduler;

    return NULL;
}


/*
 * Print command-line usage.
 */
void print_usage(const char *program)
{
    printf("\nUsage:\n");

    printf("  %s <algorithm>\n", program);
    printf("  %s rr <quantum>\n", program);

    printf("\nAvailable algorithms:\n");

    printf("  fcfs\n");
    printf("  sjf\n");
    printf("  srtf\n");
    printf("  priority\n");
    printf("  rr <quantum>\n");

    printf("\nExamples:\n");

    printf("  %s fcfs\n", program);
    printf("  %s sjf\n", program);
    printf("  %s srtf\n", program);
    printf("  %s priority\n", program);
    printf("  %s rr 2\n", program);
}


int main(int argc, char *argv[])
{
    Simulation sim;

    /*
     * ----------------------------------------
     * Check command-line arguments
     * ----------------------------------------
     */

    if (argc < 2)
    {
        printf("Error: Scheduling algorithm not specified.\n");

        print_usage(argv[0]);

        return 1;
    }


    /*
     * ----------------------------------------
     * Round Robin
     * ----------------------------------------
     *
     * RR requires:
     *
     * ./scheduler rr <quantum>
     */

    if (strcmp(argv[1], "rr") == 0)
    {
        if (argc != 3)
        {
            printf(
                "Error: Round Robin requires a time quantum.\n"
            );

            printf("\nExample:\n");
            printf("  %s rr 2\n", argv[0]);

            return 1;
        }

        char *endptr;

        long quantum =
            strtol(argv[2], &endptr, 10);


        /*
         * Check that quantum is actually a number.
         */
        if (*endptr != '\0')
        {
            printf(
                "Error: Invalid time quantum '%s'.\n",
                argv[2]
            );

            return 1;
        }


        /*
         * Quantum must be positive.
         */
        if (quantum <= 0)
        {
            printf(
                "Error: Time quantum must be greater than 0.\n"
            );

            return 1;
        }


        /*
         * Store quantum in Simulation.
         */
        initialize_simulation(&sim);

        sim.time_quantum = (int)quantum;
    }
    else
    {
        /*
         * ----------------------------------------
         * Other algorithms
         * ----------------------------------------
         */

        if (argc != 2)
        {
            printf("Error: Too many arguments.\n");

            print_usage(argv[0]);

            return 1;
        }

        initialize_simulation(&sim);
    }


    /*
     * ----------------------------------------
     * Select scheduler
     * ----------------------------------------
     */

    /*
 * ----------------------------------------
 * Select scheduler and scheduling behavior
 * ----------------------------------------
 */

        SchedulerFunction scheduler =
            get_scheduler(argv[1]);

        if (scheduler == NULL)
        {
            printf(
                "Error: Unknown scheduling algorithm '%s'.\n",
                argv[1]
            );

            print_usage(argv[0]);

            return 1;
        }


        /*
         * Configure whether the scheduler is preemptive.
         *
         * FCFS     -> non-preemptive
         * SJF      -> non-preemptive
         * SRTF     -> preemptive
         * Priority -> non-preemptive
         * RR       -> preemptive through quantum
         */

        if (strcmp(argv[1], "srtf") == 0)
        {
            sim.preemptive = 1;
        }
        else
        {
            sim.preemptive = 0;
        }

    /*
     * ----------------------------------------
     * Add processes
     * ----------------------------------------
     *
     * PID   Arrival   Burst   Priority
     *
     * P1       0        8        2
     * P2       1        4        1
     * P3       2        2        3
     * P4       3        5        2
     */

add_process(&sim, 1, 0, 8, 1);
add_process(&sim, 2, 1, 3, 4);
add_process(&sim, 3, 2, 2, 5);
add_process(&sim, 4, 3, 4, 3);


/*
     * ----------------------------------------
     * Display configuration
     * ----------------------------------------
     */

    printf("\n");
    printf("========================================\n");
    printf("       CPU SCHEDULING SIMULATOR\n");
    printf("========================================\n");

    printf("Algorithm    : %s\n", argv[1]);

    if (strcmp(argv[1], "rr") == 0)
    {
        printf(
            "Time Quantum : %d\n",
            sim.time_quantum
        );
    }

    printf("========================================\n");


    /*
     * ----------------------------------------
     * Run simulation
     * ----------------------------------------
     */

    run_simulation(&sim, scheduler);


    return 0;
}
