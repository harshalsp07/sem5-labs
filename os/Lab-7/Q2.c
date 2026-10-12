#include <stdio.h>
#include <stdbool.h>

#define READ  (1 << 0)
#define WRITE (1 << 1)
#define EXEC  (1 << 2)

typedef struct {
    unsigned int base;
    unsigned int limit;
    unsigned char perms;
    int min_ring;
} ProtectedSegment;

ProtectedSegment table[2] = {
    {1000, 500, READ | EXEC, 0},
    {3000, 800, READ | WRITE, 3}
};

bool access_segment(int seg, unsigned int offset, unsigned char req_perm, int current_cpu_ring) {
    if (current_cpu_ring > table[seg].min_ring) {
        printf("TRAP: Privilege Violation\n");
        return false;
    }
    if ((table[seg].perms & req_perm) != req_perm) {
        printf("[TRAP] Protection Violation!\n");
        return false;
    }
    if (offset >= table[seg].limit) {
        printf("[TRAP] Segmentation Fault!\n");
        return false;
    }
    printf("SUCCESS\n");
    return true;
}

int main() {
    access_segment(0, 100, READ, 3);
    access_segment(0, 100, READ, 0);
    return 0;
}
