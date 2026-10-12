#include "process.h"

void initialize_process(Process *p)
{
    p->remaining_time = p->burst_time;

    p->start_time = -1;
    p->completion_time = -1;

    p->waiting_time = 0;
    p->turnaround_time = 0;
    p->response_time = -1;

    p->state = NEW;
}
