#include <stdio.h>

#define MAX 100

int main(void)
{
    int n, f, ref[MAX], frames[MAX];
    int faults = 0;

    printf("Enter number of references: ");
    scanf("%d", &n);
    printf("Enter the reference string: ");
    for (int i = 0; i < n; i++)
        scanf("%d", &ref[i]);
    printf("Enter number of frames: ");
    scanf("%d", &f);

    for (int i = 0; i < f; i++)
        frames[i] = -1;

    printf("\nRef\tFrames\t\tResult\n");
    for (int i = 0; i < n; i++) {
        int page = ref[i];
        int hit = 0;

        for (int j = 0; j < f; j++) {
            if (frames[j] == page) {
                hit = 1;
                break;
            }
        }

        if (!hit) {
            int victim = -1;

            for (int j = 0; j < f; j++) {
                if (frames[j] == -1) {
                    victim = j;
                    break;
                }
            }

            if (victim == -1) {
                int farthest = -1;
                for (int j = 0; j < f; j++) {
                    int next_use = n;
                    for (int k = i + 1; k < n; k++) {
                        if (ref[k] == frames[j]) {
                            next_use = k;
                            break;
                        }
                    }
                    if (next_use > farthest) {
                        farthest = next_use;
                        victim = j;
                    }
                }
            }

            frames[victim] = page;
            faults++;
        }

        printf("%d\t", page);
        for (int j = 0; j < f; j++) {
            if (frames[j] == -1)
                printf("- ");
            else
                printf("%d ", frames[j]);
        }
        printf("\t\t%s\n", hit ? "Hit" : "Fault");
    }

    printf("\nPage faults: %d\n", faults);
    printf("Page hits:   %d\n", n - faults);
    return 0;
}
