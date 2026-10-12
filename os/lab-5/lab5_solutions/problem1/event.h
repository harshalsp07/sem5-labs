#ifndef EVENT_H
#define EVENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#define SHM_FILE "/tmp/os_lab5_event_shmfile"
#define PROJECT_ID 65

#define MAX_TYPE_LEN 64
#define MAX_MSG_LEN 256

typedef struct {
    int event_id;
    char event_type[MAX_TYPE_LEN];
    char event_message[MAX_MSG_LEN];
    char timestamp[32];

    /* Synchronization flags */
    int has_event;   /* 1 = new event waiting for monitor, 0 = processed/empty */
    int terminated;  /* 1 = generator has finished */
} SharedEventBuffer;

/* Helper to ensure the key token file exists */
static inline void ensure_shm_file_exists(const char *filename) {
    int fd = open(filename, O_CREAT | O_RDWR, 0666);
    if (fd >= 0) {
        close(fd);
    }
}

#endif /* EVENT_H */
