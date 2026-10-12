#include <stdio.h>

#define MAX 100

int main(void)
{
    int n, f, ref[MAX], frames[MAX], last_used[MAX];
    int faults = 0;

    printf("Enter number of references: ");
    scanf("%d", &n);
    printf("Enter the reference string: ");
    for (int i = 0; i < n; i++)
        scanf("%d", &ref[i]);
    printf("Enter number of frames: ");
    scanf("%d", &f);

    for (int i = 0; i < f; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }

    printf("\nRef\tFrames\t\tResult\n");
    for (int i = 0; i < n; i++) {
        int page = ref[i];
        int hit = 0;

        for (int j = 0; j < f; j++) {
            if (frames[j] == page) {
                hit = 1;
                last_used[j] = i;
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
                victim = 0;
                for (int j = 1; j < f; j++)
                    if (last_used[j] < last_used[victim])
                        victim = j;
            }

            frames[victim] = page;
            last_used[victim] = i;
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
