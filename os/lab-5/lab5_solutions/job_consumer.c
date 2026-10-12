#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define MAX_JOBS 5

typedef struct {
    int job_id;
    int priority;
    int cpu_burst_time;
} Job;

typedef struct {
    int job_count;
    Job jobs[MAX_JOBS];
} SharedJobs;

int main(void)
{
    key_t key = ftok("shmfile", 65);
    if (key == (key_t)-1) {
        perror("ftok");
        return 1;
    }

    int shmid = shmget(key, sizeof(SharedJobs), 0666);
    if (shmid == -1) {
        perror("shmget: run job_producer first");
        return 1;
    }

    SharedJobs *shared = (SharedJobs *)shmat(shmid, NULL, 0);
    if (shared == (void *)-1) {
        perror("shmat");
        return 1;
    }

    if (shared->job_count < 1 || shared->job_count > MAX_JOBS) {
        fprintf(stderr, "Shared memory contains an invalid job count.\n");
        shmdt(shared);
        return 1;
    }

    int remaining = shared->job_count;
    printf("Scheduler running %d job(s) by priority (higher number first).\n",
           remaining);
    while (remaining > 0) {
        int selected = -1;
        for (int i = 0; i < shared->job_count; i++) {
            if (shared->jobs[i].cpu_burst_time > 0 &&
                (selected == -1 ||
                 shared->jobs[i].priority > shared->jobs[selected].priority)) {
                selected = i;
            }
        }

        Job *job = &shared->jobs[selected];
        printf("Executing Job %d: priority=%d, CPU burst=%d\n",
               job->job_id, job->priority, job->cpu_burst_time);
        sleep((unsigned int)job->cpu_burst_time);
        job->cpu_burst_time = 0;
        remaining--;
    }

    printf("All jobs completed.\n");
    if (shmdt(shared) == -1) {
        perror("shmdt");
        return 1;
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        return 1;
    }
    return 0;
}
