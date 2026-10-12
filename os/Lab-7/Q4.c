#include <stdio.h>
#include <string.h>

char RAM[1000];

typedef struct {
    int id;
    int base;
    int size;
} ActiveSegment;

ActiveSegment segs[2] = {
    {0, 200, 6},
    {1, 600, 6}
};
int count = 2;

void compact_memory() {
    int current_base = 0;
    printf("--- Starting Compaction ---\n");
    for (int i = 0; i < count; i++) {
        int old_base = segs[i].base;
        memmove(&RAM[current_base], &RAM[old_base], segs[i].size);
        if (old_base != current_base) {
            memset(&RAM[old_base], 0, segs[i].size);
        }
        printf("Relocating Seg %d: Old Base = %d -> New Base = %d\n", 
               segs[i].id, old_base, current_base);
        segs[i].base = current_base;
        current_base += segs[i].size;
    }
    printf("Compaction Complete. Contiguous Free Space starts at Base: %d\n", current_base);
}

int main() {
    memcpy(&RAM[200], "HELLO", 6);
    memcpy(&RAM[600], "WORLD", 6);
    compact_memory();
    for (int i = 0; i < count; i++) {
        printf("Seg %d: Compacted Base = Base %d, Updated Memory Offset: RAM[%d..%d] = \"%s\"\n",
               segs[i].id, segs[i].base, segs[i].base, segs[i].base + (int)strlen(&RAM[segs[i].base]) - 1, &RAM[segs[i].base]);
    }
    return 0;
}
