#include <stdio.h>
#include <string.h>

#define MAX 105

int main(void)
{
    int k, n;
    char ref[MAX][35];

    scanf("%d", &k);
    scanf("%d", &n);
    for (int i = 0; i < n; i++)
        scanf("%s", ref[i]);

    char frames[MAX][35];
    int put[MAX];
    int trips = 0;

    for (int i = 0; i < k; i++) {
        frames[i][0] = '\0';
        put[i] = -1;
    }

    for (int i = 0; i < n; i++) {
        int hit = 0;
        for (int j = 0; j < k; j++) {
            if (frames[j][0] != '\0' && strcmp(frames[j], ref[i]) == 0) {
                hit = 1;
                break;
            }
        }

        if (!hit) {
            trips++;
            int victim = -1;
            for (int j = 0; j < k; j++) {
                if (frames[j][0] == '\0') {
                    victim = j;
                    break;
                }
            }

            if (victim != -1) {
                printf("Trip %d: take %s\n", trips, ref[i]);
            } else {
                int farthest = -1;
                for (int j = 0; j < k; j++) {
                    int next_use = n;
                    for (int l = i + 1; l < n; l++) {
                        if (strcmp(ref[l], frames[j]) == 0) {
                            next_use = l;
                            break;
                        }
                    }
                    if (next_use > farthest) {
                        farthest = next_use;
                        victim = j;
                    } else if (next_use == farthest && next_use == n) {
                        if (put[j] < put[victim])
                            victim = j;
                    }
                }
                printf("Trip %d: take %s, return %s\n", trips, ref[i], frames[victim]);
            }

            strcpy(frames[victim], ref[i]);
            put[victim] = trips;
        }
    }

    printf("Total trips: %d\n", trips);

    char lframes[MAX][35];
    int last[MAX];
    int ltrips = 0;

    for (int i = 0; i < k; i++) {
        lframes[i][0] = '\0';
        last[i] = -1;
    }

    for (int i = 0; i < n; i++) {
        int hit = 0;
        for (int j = 0; j < k; j++) {
            if (lframes[j][0] != '\0' && strcmp(lframes[j], ref[i]) == 0) {
                hit = 1;
                last[j] = i;
                break;
            }
        }

        if (!hit) {
            ltrips++;
            int victim = -1;
            for (int j = 0; j < k; j++) {
                if (lframes[j][0] == '\0') {
                    victim = j;
                    break;
                }
            }
            if (victim == -1) {
                victim = 0;
                for (int j = 1; j < k; j++) {
                    if (last[j] < last[victim])
                        victim = j;
                }
            }
            strcpy(lframes[victim], ref[i]);
            last[victim] = i;
        }
    }

    printf("Trips if returning least recently used tool: %d\n", ltrips);

    return 0;
}
