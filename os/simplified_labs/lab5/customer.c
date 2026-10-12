/*
 * Lab 5 Solutions - Problem 2: Sleeping Barber (Customer Process)
 *
 * Purpose:
 *   Simulates customers entering the barber shop:
 *   - If barber is sleeping, wake him up and get haircut immediately.
 *   - If barber is busy and chairs are free, sit down and wait.
 *   - If chairs are full, leave the shop (balking).
 *
 * Compilation:
 *   gcc customer.c -o customer
 *
 * Usage:
 *   ./customer
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 2222
#define NUM_CHAIRS 3

#define BARBER_SLEEPING 0
#define BARBER_BUSY     1

typedef struct {
    int queue[NUM_CHAIRS];
    int front;
    int rear;
    int waiting_count;
    int barber_state;
    int current_customer;
    int stop;
} BarberShop;

int main() {
    int shmid = shmget(SHM_KEY, sizeof(BarberShop), 0666);
    if (shmid < 0) {
        perror("shmget failed (Make sure ./barber is running first)");
        return 1;
    }

    BarberShop *shop = (BarberShop *)shmat(shmid, NULL, 0);
    if (shop == (BarberShop *)-1) {
        perror("shmat failed");
        return 1;
    }

    printf("=========================================\n");
    printf(" Customer Generator Started\n");
    printf(" Enter Customer ID (-1 to close shop)\n");
    printf("=========================================\n");

    while (1) {
        int id;
        printf("\nEnter Customer ID: ");
        if (scanf("%d", &id) != 1) break;

        if (id == -1) {
            shop->stop = 1;
            printf("Instructed barber to finish waiting customers and close.\n");
            break;
        }

        // Check barber status
        if (shop->barber_state == BARBER_SLEEPING && shop->current_customer == 0) {
            // Wake barber up
            shop->current_customer = id;
            shop->barber_state = BARBER_BUSY;
            printf("-> Customer %d woke up the sleeping barber! Haircut started.\n", id);
        } else if (shop->waiting_count < NUM_CHAIRS) {
            // Take a waiting chair
            shop->queue[shop->rear] = id;
            shop->rear = (shop->rear + 1) % NUM_CHAIRS;
            shop->waiting_count++;
            printf("-> Barber is busy. Customer %d sat down in a chair (Waiting: %d/%d)\n",
                   id, shop->waiting_count, NUM_CHAIRS);
        } else {
            // No empty chairs
            printf("-> All %d chairs are full! Customer %d left the shop.\n", NUM_CHAIRS, id);
        }
    }

    shmdt(shop);
    return 0;
}
