#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int start;
    int size;
    bool is_free;
} Block;

#define MAX_BLOCKS 10
Block memory[MAX_BLOCKS] = {
    {0,   200, false},
    {200, 300, true},
    {500, 200, false},
    {700, 200, true}
};
int block_count = 4;

int best_fit_allocate(int req_size) {
    int best_idx = -1;
    for (int i = 0; i < block_count; i++) {
        if (memory[i].is_free && memory[i].size >= req_size) {
            if (best_idx == -1 || memory[i].size < memory[best_idx].size) {
                best_idx = i;
            }
        }
    }
    if (best_idx != -1) {
        int chosen_base = memory[best_idx].start;
        int chosen_size = memory[best_idx].size;
        int leftover_size = chosen_size - req_size;
        int leftover_base = chosen_base + req_size;

        memory[best_idx].size = req_size;
        memory[best_idx].is_free = false;

        if (leftover_size > 0 && block_count < MAX_BLOCKS) {
            for (int i = block_count; i > best_idx + 1; i--) {
                memory[i] = memory[i - 1];
            }
            memory[best_idx + 1].start = leftover_base;
            memory[best_idx + 1].size = leftover_size;
            memory[best_idx + 1].is_free = true;
            block_count++;
        }
        printf("[ALLOC] Segment of size %d allocated at Base: %d\n", req_size, chosen_base);
        printf("Chosen Hole Base: Base %d (Size %d)\n", chosen_base, chosen_size);
        if (leftover_size > 0) {
            printf("New Leftover Hole: Base %d (Size %d)\n", leftover_base, leftover_size);
        }
        return chosen_base;
    }
    printf("[FAILED] No contiguous free block of size %d available\n", req_size);
    return -1;
}

int main() {
    best_fit_allocate(150);
    return 0;
}
