#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "barber.h"

static int g_shmid = -1;
static BarberShop *g_shop = NULL;

void handle_signal(int sig) {
    (void)sig;
    if (g_shop != NULL && g_shop != (void *)-1) {
        shmdt(g_shop);
    }
    if (g_shmid != -1) {
        shmctl(g_shmid, IPC_RMID, NULL);
    }
    printf("\nShared memory removed. Barber exiting.\n");
    exit(0);
}

int main(void) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    ensure_barber_shm_file();

    key_t key = ftok(BARBER_SHM_FILE, BARBER_PROJECT_ID);
    if (key == (key_t)-1) {
        perror("ftok");
        exit(1);
    }

    int shmid = shmget(key, sizeof(BarberShop), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    g_shmid = shmid;

    BarberShop *shop = (BarberShop *)shmat(shmid, NULL, 0);
    if (shop == (void *)-1) {
        perror("shmat");
        exit(1);
    }
    g_shop = shop;

    acquire_lock(shop);
    shop->front = 0;
    shop->rear = 0;
    shop->waiting_count = 0;
    shop->barber_state = BARBER_SLEEPING;
    shop->current_customer_id = 0;
    shop->stop_generator = 0;
    release_lock(shop);

    printf("--- Barber Shop ---\n");
    printf("Shop open with %d waiting chairs.\n", NUM_CHAIRS);
    printf("Barber is sleeping (waiting for customers)...\n\n");

    int total_served = 0;

    while (1) {
        acquire_lock(shop);

        if (shop->barber_state == BARBER_BUSY && shop->current_customer_id != 0) {
            int cust = shop->current_customer_id;
            release_lock(shop);

            printf("Barber is cutting hair for customer %d...\n", cust);
            sleep(2);
            total_served++;
            printf("Finished haircut for customer %d.\n", cust);

            acquire_lock(shop);
            shop->current_customer_id = 0;

            if (shop->waiting_count > 0) {
                int next_cust = shop->waiting_queue[shop->front];
                shop->front = (shop->front + 1) % NUM_CHAIRS;
                shop->waiting_count--;
                shop->current_customer_id = next_cust;
                shop->barber_state = BARBER_BUSY;

                printf("Serving waiting customer %d from chair (%d still waiting).\n",
                       next_cust, shop->waiting_count);
            } else {
                shop->barber_state = BARBER_SLEEPING;
                printf("No customers waiting. Barber goes to sleep.\n\n");
            }
            release_lock(shop);
        } else {
            int stop = shop->stop_generator;
            int waiting = shop->waiting_count;
            int current = shop->current_customer_id;
            release_lock(shop);

            if (stop == 1 && waiting == 0 && current == 0) {
                printf("\nCustomer generation stopped. Closing shop.\n");
                printf("Total customers served: %d\n", total_served);
                break;
            }

            usleep(100000);
        }
    }

    shmdt(g_shop);
    shmctl(g_shmid, IPC_RMID, NULL);
    printf("Shared memory cleaned up. Barber finished.\n");
    return 0;
}
