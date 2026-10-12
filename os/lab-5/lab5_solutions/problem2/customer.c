#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "barber.h"

int main(void) {
    ensure_barber_shm_file();

    key_t key = ftok(BARBER_SHM_FILE, BARBER_PROJECT_ID);
    if (key == (key_t)-1) {
        perror("ftok");
        exit(1);
    }

    int shmid = shmget(key, sizeof(BarberShop), 0666);
    if (shmid == -1) {
        perror("shmget (run barber process first)");
        exit(1);
    }

    BarberShop *shop = (BarberShop *)shmat(shmid, NULL, 0);
    if (shop == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    printf("--- Customer Generator ---\n");

    char buf[64];
    while (1) {
        printf("\nEnter Customer ID (-1 to stop): ");
        if (!fgets(buf, sizeof(buf), stdin)) {
            break;
        }

        int cust_id = atoi(buf);

        if (cust_id == -1) {
            acquire_lock(shop);
            shop->stop_generator = 1;
            release_lock(shop);
            printf("Stopping customer generator...\n");
            break;
        }

        if (cust_id <= 0) {
            printf("Please enter a valid positive ID or -1 to stop.\n");
            continue;
        }

        acquire_lock(shop);

        if (shop->barber_state == BARBER_SLEEPING && shop->waiting_count == 0 && shop->current_customer_id == 0) {
            // Wake barber
            shop->barber_state = BARBER_BUSY;
            shop->current_customer_id = cust_id;
            printf("Customer %d: Barber was sleeping. Woke up barber and started haircut.\n", cust_id);
        } else if (shop->waiting_count < NUM_CHAIRS) {
            // Sit in chair
            shop->waiting_queue[shop->rear] = cust_id;
            shop->rear = (shop->rear + 1) % NUM_CHAIRS;
            shop->waiting_count++;
            printf("Customer %d: Barber is busy. Sitting in waiting chair (%d/%d chairs occupied).\n",
                   cust_id, shop->waiting_count, NUM_CHAIRS);
        } else {
            // Chairs full
            printf("Customer %d: All %d waiting chairs are full! Customer leaves.\n",
                   cust_id, NUM_CHAIRS);
        }

        release_lock(shop);
    }

    shmdt(shop);
    return 0;
}
