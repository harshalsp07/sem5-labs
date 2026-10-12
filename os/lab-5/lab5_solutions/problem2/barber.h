#ifndef BARBER_H
#define BARBER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <fcntl.h>

#define BARBER_SHM_FILE "/tmp/os_lab5_barber_shmfile"
#define BARBER_PROJECT_ID 75

#define NUM_CHAIRS 3

#define BARBER_SLEEPING 0
#define BARBER_BUSY     1

typedef struct {
    int waiting_queue[NUM_CHAIRS];
    int front;
    int rear;
    int waiting_count;

    int barber_state;        /* BARBER_SLEEPING or BARBER_BUSY */
    int current_customer_id; /* customer ID currently getting haircut, 0 if none */

    int lock;                /* atomic spinlock flag */
    int stop_generator;      /* set to 1 when customer generator finishes (-1) */
} BarberShop;

static inline void ensure_barber_shm_file(void) {
    int fd = open(BARBER_SHM_FILE, O_CREAT | O_RDWR, 0666);
    if (fd >= 0) {
        close(fd);
    }
}

static inline void acquire_lock(BarberShop *shop) {
    while (__sync_lock_test_and_set(&shop->lock, 1)) {
        usleep(1000); // 1ms
    }
}

static inline void release_lock(BarberShop *shop) {
    __sync_lock_release(&shop->lock);
}

#endif /* BARBER_H */
