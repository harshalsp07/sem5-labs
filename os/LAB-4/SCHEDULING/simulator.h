#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "process.h"
#include "queue.h"

#define MAX_PROCESSES 100
#define MAX_HISTORY 1000


typedef struct
{
    int pid;

    int start_time;
    int end_time;

} ExecutionRecord;


typedef struct
{
    Process processes[MAX_PROCESSES];

    int process_count;

    int current_time;

    int completed;

    int running_pid;

    int current_process;

    int context_switches;

    Queue ready_queue;

    ExecutionRecord history[MAX_HISTORY];

    int history_count;

    int time_quantum;
    int quantum_used;
    int preemptive;

} Simulation;
/*
 * Scheduler function.
 *
 * Returns the index of the process to execute.
 *
 * Returns -1 when no process should run.
 */
typedef int (*SchedulerFunction)(Simulation *sim);


/*
 * Simulation management.
 */
void initialize_simulation(Simulation *sim);

void add_process(
    Simulation *sim,
    int pid,
    int arrival,
    int burst,
    int priority
);


/*
 * Run the simulation.
 */
void run_simulation(
    Simulation *sim,
    SchedulerFunction scheduler
);


/*
 * Common helper functions available
 * to scheduling algorithms.
 */
void add_to_ready_queue(
    Simulation *sim,
    int process_index
);


void add_execution_record(
    Simulation *sim,
    int process_index,
    int start,
    int end
);

#endif
