#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int segment_id;
    unsigned int base;
    unsigned int limit;
    bool is_downward;
} Segment;

#define MAX_SEGMENTS 3
Segment segment_table[MAX_SEGMENTS] = {
    {0, 1400, 1000, false},
    {1, 6300, 400, false},
    {2, 4700, 400, true}
};

bool translate(int seg_num, unsigned int offset, unsigned int *phys_addr) {
    if (seg_num < 0 || seg_num >= MAX_SEGMENTS) return false;
    
    if (offset >= segment_table[seg_num].limit) return false;
    
    if (segment_table[seg_num].is_downward) {
        *phys_addr = segment_table[seg_num].base - offset;
    } else {
        *phys_addr = segment_table[seg_num].base + offset;
    }
    return true;
}

int main() {
    unsigned int phys;
    if (translate(2, 100, &phys)) 
        printf("Seg 2, Offset 100 -> Physical Address: %u\n", phys);
    if (!translate(2, 500, &phys)) 
        printf("Seg 2, Offset 500 -> SEGMENTATION FAULT\n");
    return 0;
}
