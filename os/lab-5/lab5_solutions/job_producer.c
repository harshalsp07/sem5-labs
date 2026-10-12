#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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
    int job_count;

    printf("Enter the number of jobs (1-%d): ", MAX_JOBS);
    if (scanf("%d", &job_count) != 1 || job_count < 1 || job_count > MAX_JOBS) {
        fprintf(stderr, "The number of jobs must be between 1 and %d.\n", MAX_JOBS);
        return 1;
    }

    Job jobs[MAX_JOBS];
    printf("Enter each job as: job_id priority cpu_burst_time\n");
    for (int i = 0; i < job_count; i++) {
        if (scanf("%d %d %d", &jobs[i].job_id, &jobs[i].priority,
                  &jobs[i].cpu_burst_time) != 3 || jobs[i].cpu_burst_time < 1) {
            fprintf(stderr, "Invalid job. CPU burst time must be positive.\n");
            return 1;
        }
    }

    key_t key = ftok("shmfile", 65);
    if (key == (key_t)-1) {
        perror("ftok");
        return 1;
    }

    int shmid = shmget(key, sizeof(SharedJobs), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    SharedJobs *shared = (SharedJobs *)shmat(shmid, NULL, 0);
    if (shared == (void *)-1) {
        perror("shmat");
        return 1;
    }

    shared->job_count = job_count;
    for (int i = 0; i < job_count; i++) {
        shared->jobs[i] = jobs[i];
    }

    printf("Generated %d job(s) in shared memory.\n", job_count);
    if (shmdt(shared) == -1) {
        perror("shmdt");
        return 1;
    }
    return 0;
}
