#include "../simulator.h"

int srtf_scheduler(Simulation *sim)
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
            if( sim->processes[selected].remaining_time <= 3 && p->remaining_time <
                sim->processes[selected].remaining_time){
                selected =i; 
            }
            else if (selected == -1 ||
                p->remaining_time <
                sim->processes[selected].remaining_time)
            {
                selected = i;
            }
        }
    }

    return selected;
}
