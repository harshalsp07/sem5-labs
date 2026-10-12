/*
 * Lab 5 Solutions - Problem 2: Sleeping Barber (Barber Process)
 *
 * Purpose:
 *   Simulates the barber in the Sleeping Barber problem:
 *   - Sleeps when no customers are waiting.
 *   - Woken up when a customer arrives.
 *   - Cuts hair for waiting customers in the chairs queue.
 *
 * Compilation:
 *   gcc barber.c -o barber
 *
 * Usage:
 *   ./barber
 *   (Then in another terminal, run ./customer)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#define SHM_KEY 2222
#define NUM_CHAIRS 3

#define BARBER_SLEEPING 0
#define BARBER_BUSY     1

typedef struct {
    int queue[NUM_CHAIRS];
    int front;
    int rear;
    int waiting_count;
    int barber_state;        // BARBER_SLEEPING or BARBER_BUSY
    int current_customer;    // ID of customer currently in barber's chair
    int stop;                // 1 = shutdown
} BarberShop;

static int g_shmid = -1;
static BarberShop *g_shop = NULL;

void handle_exit(int sig) {
    (void)sig;
    if (g_shop != NULL && g_shop != (BarberShop *)-1) shmdt(g_shop);
    if (g_shmid != -1) shmctl(g_shmid, IPC_RMID, NULL);
    printf("\nShared memory cleaned. Barber shop closed.\n");
    exit(0);
}

int main() {
    signal(SIGINT, handle_exit);

    int shmid = shmget(SHM_KEY, sizeof(BarberShop), 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }
    g_shmid = shmid;

    BarberShop *shop = (BarberShop *)shmat(shmid, NULL, 0);
    if (shop == (BarberShop *)-1) {
        perror("shmat failed");
        return 1;
    }
    g_shop = shop;

    // Initialize barbershop state
    shop->front = 0;
    shop->rear = 0;
    shop->waiting_count = 0;
    shop->barber_state = BARBER_SLEEPING;
    shop->current_customer = 0;
    shop->stop = 0;

    printf("=========================================\n");
    printf(" Barber Shop Open! (Chairs available: %d)\n", NUM_CHAIRS);
    printf(" Barber is currently SLEEPING...\n");
    printf(" Run './customer' to send customers.\n");
    printf("=========================================\n");

    while (1) {
        // If there is an active customer getting a haircut
        if (shop->current_customer != 0) {
            shop->barber_state = BARBER_BUSY;
            printf("\n[BARBER] Cutting hair of Customer %d...\n", shop->current_customer);
            sleep(2); // Simulate haircut time
            printf("[BARBER] Finished haircut of Customer %d.\n", shop->current_customer);
            shop->current_customer = 0;

            // Check if another customer is waiting in the chairs
            if (shop->waiting_count > 0) {
                shop->current_customer = shop->queue[shop->front];
                shop->front = (shop->front + 1) % NUM_CHAIRS;
                shop->waiting_count--;
                printf("[BARBER] Called next waiting Customer %d from chair (waiting left: %d)\n",
                       shop->current_customer, shop->waiting_count);
            } else {
                shop->barber_state = BARBER_SLEEPING;
                printf("[BARBER] No waiting customers. Barber going back to SLEEP zzz...\n");
            }
        }

        // Exit condition
        if (shop->stop == 1 && shop->waiting_count == 0 && shop->current_customer == 0) {
            printf("\nAll customers served. Barber closing shop.\n");
            break;
        }

        usleep(50000); // 50ms check
    }

    shmdt(shop);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
