#include "../simulator.h"

int priority_scheduler(Simulation *sim)
{
    int selected = -1;

    for (int i = 0; i < sim->process_count; i++) {

        Process *p = &sim->processes[i];
         int waiting_time = sim->current_time-sim->processes[i].arrival_time;
         int effective_priority = p->priority - waiting_time / 3;
        if (p->state == READY) {

            if (selected == -1 ||
                effective_priority <
                sim->processes[selected].priority) {

                selected = i;
            }
        }
    }

    return selected;
}
