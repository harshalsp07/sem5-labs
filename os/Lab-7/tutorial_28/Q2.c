#include <stdio.h>
#include <stdbool.h>

#define READ  (1 << 0) // 001
#define WRITE (1 << 1) // 010
#define EXEC  (1 << 2) // 100

typedef struct {
    unsigned int base;
    unsigned int limit;
    unsigned char perms;
} ProtectedSegment;

ProtectedSegment table[2] = {
    {1000, 500, READ | EXEC}, // Code: R-X
    {3000, 800, READ | WRITE} // Data: RW-
};

bool access_segment(int seg, unsigned int offset, unsigned char req_perm) {
    // Permission check using bitmask
    if ((table[seg].perms & req_perm) != req_perm) {
        printf("[TRAP] Protection Violation!\n");
        return false;
    }
    if (offset >= table[seg].limit) {
        printf("[TRAP] Segmentation Fault!\n");
        return false;
    }
    printf("Access Granted -> Physical: %u\n", table[seg].base + offset);
    return true;
}

int main() {
    access_segment(0, 100, EXEC);  // Allowed
    access_segment(0, 100, WRITE); // Blocked (Code isn't writable)
    return 0;
}
