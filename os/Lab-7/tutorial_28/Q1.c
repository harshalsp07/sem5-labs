#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int segment_id;
    unsigned int base;
    unsigned int limit;
} Segment;

#define MAX_SEGMENTS 3
Segment segment_table[MAX_SEGMENTS] = {
    {0, 1400, 1000}, // Code
    {1, 6300, 400},  // Data
    {2, 4300, 400}   // Stack
};

bool translate(int seg_num, unsigned int offset, unsigned int *phys_addr) {
    if (seg_num < 0 || seg_num >= MAX_SEGMENTS) return false;
    
    // Bounds check
    if (offset >= segment_table[seg_num].limit) return false;
    
    *phys_addr = segment_table[seg_num].base + offset;
    return true;
}

int main() {
    unsigned int phys;
    // Valid access
    if (translate(0, 500, &phys)) 
        printf("Seg 0, Offset 500 -> Physical Address: %u\n", phys);
    // Invalid access (out of bounds)
    if (!translate(1, 500, &phys)) 
        printf("Seg 1, Offset 500 -> SEGMENTATION FAULT (Exceeds limit 400)\n");
    return 0;
}
