#include "../simulator.h"

int rr_scheduler(Simulation *sim)
{
    return queue_pop(&sim->ready_queue);
}