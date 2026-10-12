#include <stdio.h>

typedef struct {
    int id;
    int base;
    int size;
} ActiveSegment;

ActiveSegment segs[3] = {
    {0, 100, 200}, // Gap between 0-100
    {1, 400, 150}, // Gap between 300-400
    {2, 700, 100}  // Gap between 550-700
};
int count = 3;

void compact_memory() {
    int current_base = 0;
    printf("--- Starting Compaction ---\n");
    for (int i = 0; i < count; i++) {
        printf("Relocating Seg %d: Old Base = %d -> New Base = %d\n", 
               segs[i].id, segs[i].base, current_base);
        segs[i].base = current_base;
        current_base += segs[i].size;
    }
    printf("Compaction Complete. Contiguous Free Space starts at Base: %d\n", current_base);
}

int main() {
    compact_memory();
    return 0;
}
