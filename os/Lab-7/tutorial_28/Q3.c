#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int start;
    int size;
    bool is_free;
} Block;

#define MAX_BLOCKS 10
Block memory[MAX_BLOCKS] = {
    {0,   200, false}, // Occupied (Seg 0)
    {200, 300, true},  // Free Hole A
    {500, 100, false}, // Occupied (Seg 1)
    {600, 400, true}   // Free Hole B
};
int block_count = 4;

int allocate_first_fit(int seg_size) {
    for (int i = 0; i < block_count; i++) {
        // Find FIRST free block big enough
        if (memory[i].is_free && memory[i].size >= seg_size) {
            memory[i].is_free = false;
            printf("[ALLOC] Segment of size %d allocated at Base: %d\n", seg_size, memory[i].start);
            return memory[i].start;
        }
    }
    printf("[FAILED] No contiguous free block of size %d available (External Fragmentation!)\n", seg_size);
    return -1;
}

int main() {
    printf("=== Tutorial 3: First-Fit Segment Allocation ===\n\n");

    printf("Request 1: Allocate segment of size 250...\n");
    allocate_first_fit(250); // Fits into first free block at Base 200 (size 300)

    printf("\nRequest 2: Allocate segment of size 500...\n");
    allocate_first_fit(500); // Fails! Total free = 450 (after req 1), but fragmented.

    return 0;
}
