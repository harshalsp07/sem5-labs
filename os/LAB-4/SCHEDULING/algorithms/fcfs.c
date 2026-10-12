#include <stdio.h>
#include "../simulator.h"

int fcfs_scheduler(Simulation *sim)
{
    int selected = -1;

    for (int i = 0; i < sim->process_count; i++) {

        Process *p = &sim->processes[i];

        if (p->state == READY) {

            if (selected == -1 ||
                p->arrival_time <
                sim->processes[selected].arrival_time) {

                selected = i;
            }
        }
    }

    return selected;
}
