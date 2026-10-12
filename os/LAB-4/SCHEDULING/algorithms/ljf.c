#include "../simulator.h"

int ljf_scheduler(Simulation *sim)
{
    int selected = -1;

    for (int i = 0;
         i < sim->process_count;
         i++)
    {
        Process *p =
            &sim->processes[i];

        if (p->state == READY)
        {
            if (selected == -1 ||
                p->burst_time >
                sim->processes[selected].burst_time)
            {
                selected = i;
            }
        }
    }

    return selected;
}
