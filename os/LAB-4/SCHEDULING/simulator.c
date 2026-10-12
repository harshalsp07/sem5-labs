#include <stdio.h>

#include "simulator.h"
#include "metrics.h"


void initialize_simulation(Simulation *sim)
{
    sim->process_count = 0;

    sim->current_time = 0;

    sim->completed = 0;

    sim->running_pid = -1;

    sim->current_process = -1;

    sim->context_switches = 0;

    sim->history_count = 0;

    sim->time_quantum = 0;

    sim->quantum_used = 0;
    sim->preemptive = 0;

    queue_init(&sim->ready_queue);
}


void add_process(
    Simulation *sim,
    int pid,
    int arrival,
    int burst,
    int priority
)
{
    if (sim->process_count >= MAX_PROCESSES)
    {
        printf("Maximum number of processes reached.\n");
        return;
    }

    Process *p =
        &sim->processes[sim->process_count];

    p->pid = pid;
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->priority = priority;

    initialize_process(p);

    sim->process_count++;
}


/*
 * Put a process into the READY queue.
 */
void add_to_ready_queue(
    Simulation *sim,
    int process_index
)
{
    Process *p =
        &sim->processes[process_index];

    if (p->state == TERMINATED)
        return;

    if (p->state != READY)
    {
    
        p->state = READY;

        queue_push(
            &sim->ready_queue,
            process_index
        );
    }
}


/*
 * Detect newly arrived processes.
 */
static void check_arrivals(Simulation *sim)
{
    for (int i = 0;
         i < sim->process_count;
         i++)
    {
        Process *p =
            &sim->processes[i];

        if (p->state == NEW &&
            p->arrival_time <= sim->current_time)
        {
            printf(
                "Time %d: P%d arrived\n",
                sim->current_time,
                p->pid
            );

            add_to_ready_queue(sim, i);
        }
    }
}


/*
 * Add execution interval to Gantt history.
 */
void add_execution_record(
    Simulation *sim,
    int process_index,
    int start,
    int end
)
{
    if (sim->history_count >= MAX_HISTORY)
        return;

    ExecutionRecord *record =
        &sim->history[sim->history_count];

    record->pid =
        sim->processes[process_index].pid;

    record->start_time = start;

    record->end_time = end;

    sim->history_count++;
}


/*
 * Check whether all processes are finished.
 */
static int simulation_complete(
    Simulation *sim
)
{
    return sim->completed ==
           sim->process_count;
}


/*
 * Execute ONE CPU time unit.
 */
static void execute_process(
    Simulation *sim,
    int index
)
{
    Process *p =
        &sim->processes[index];

    int start =
        sim->current_time;


    /*
     * First time this process gets CPU.
     */
    if (p->start_time == -1)
    {
        p->start_time =
            sim->current_time;

        p->response_time =
            p->start_time -
            p->arrival_time;
    }


    p->state = RUNNING;


    printf(
        "Time %d: P%d running\n",
        sim->current_time,
        p->pid
    );


    /*
     * Execute one CPU time unit.
     */
    p->remaining_time--;

    sim->current_time++;


    /*
     * Record execution.
     */
    add_execution_record(
        sim,
        index,
        start,
        sim->current_time
    );


    /*
     * Track RR quantum.
     */
    if (sim->time_quantum > 0)
    {
        sim->quantum_used++;
    }


    /*
     * ----------------------------------------
     * Process completed
     * ----------------------------------------
     */
    if (p->remaining_time == 0)
    {
        p->state = TERMINATED;

        p->completion_time =
            sim->current_time;

        sim->completed++;


        sim->current_process = -1;

        sim->quantum_used = 0;


        printf(
            "Time %d: P%d completed\n",
            sim->current_time,
            p->pid
        );

        return;
    }


    /*
     * ----------------------------------------
     * RR quantum expired
     * ----------------------------------------
     */
    if (sim->time_quantum > 0 &&
        sim->quantum_used >= sim->time_quantum)
    {
        printf(
            "Time %d: P%d quantum expired\n",
            sim->current_time,
            p->pid
        );


        /*
         * Put the process at the BACK
         * of the ready queue.
         */
        p->state = READY;

        queue_push(
            &sim->ready_queue,
            index
        );


        /*
         * CPU is now available.
         */
        sim->current_process = -1;


        sim->quantum_used = 0;

        return;
    }


    /*
     * ----------------------------------------
     * Process continues running
     * ----------------------------------------
     *
     * For non-RR algorithms we release
     * the process after every time unit
     * so the scheduler can make a new decision.
     */
    if (sim->preemptive)
    {
        p->state = READY;

        sim->current_process = -1;
    }
}


/*
 * Main simulation engine.
 */
void run_simulation(
    Simulation *sim,
    SchedulerFunction scheduler
)
{
    sim->current_time = 0;

    sim->completed = 0;

    sim->running_pid = -1;

    sim->current_process = -1;

    sim->context_switches = 0;

    sim->history_count = 0;

    sim->quantum_used = 0;

    queue_init(&sim->ready_queue);


    /*
     * Reset all processes.
     */
    for (int i = 0;
         i < sim->process_count;
         i++)
    {
        Process *p =
            &sim->processes[i];

        p->remaining_time =
            p->burst_time;

        p->start_time = -1;

        p->completion_time = -1;

        p->waiting_time = 0;

        p->turnaround_time = 0;

        p->response_time = -1;

        p->state = NEW;
    }


    printf("\n");
    printf("========================================\n");
    printf("         STARTING SIMULATION\n");
    printf("========================================\n\n");


    while (!simulation_complete(sim))
    {
        /*
         * ------------------------------------
         * 1. Check new arrivals
         * ------------------------------------
         */
        check_arrivals(sim);


        /*
         * ------------------------------------
         * 2. Select a process if CPU is free
         * ------------------------------------
         */
        if (sim->current_process == -1)
        {
            int selected =
                scheduler(sim);


            /*
             * No READY process.
             */
            if (selected == -1)
            {
                printf(
                    "Time %d: CPU IDLE\n",
                    sim->current_time
                );

                sim->current_time++;

                continue;
            }

                     /*
             * Count context switch.
             *
             * CPU -> first process is NOT counted.
             * Process -> different process IS counted.
             */
            if (sim->running_pid != -1 &&
                sim->running_pid != selected)
            {
                sim->context_switches++;
            }

            /*
             * New process dispatched.
             */
            sim->current_process = selected;

            /*
             * New RR time slice.
             */
            sim->quantum_used = 0;

            /*
             * Remember which process is running.
             */
            sim->running_pid = selected;
         
         
         
         
        }


        /*
         * ------------------------------------
         * 3. Execute ONE CPU tick
         * ------------------------------------
         */
        execute_process(
            sim,
            sim->current_process
        );
    }


    /*
     * ----------------------------------------
     * Calculate final metrics
     * ----------------------------------------
     */
    calculate_metrics(sim);

    print_results(sim);

    print_gantt_chart(sim);
}
