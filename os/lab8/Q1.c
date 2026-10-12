#include <stdio.h>

#define MAX 1005

int main(void)
{
    int k, n, t;
    int ref[MAX];

    scanf("%d", &k);
    scanf("%d", &n);
    for (int i = 0; i < n; i++)
        scanf("%d", &ref[i]);
    scanf("%d", &t);

    int frames[MAX];
    int next = 0, cnt = 0, tm = 0;
    for (int i = 0; i < k; i++)
        frames[i] = -1;

    for (int i = 0; i < n; i++) {
        int hit = 0;
        for (int j = 0; j < k; j++) {
            if (frames[j] == ref[i]) {
                hit = 1;
                break;
            }
        }

        if (hit) {
            tm += 1;
        } else {
            tm += 5;
            frames[next] = ref[i];
            next = (next + 1) % k;
            if (cnt < k)
                cnt++;
        }
    }

    printf("Total time with %d slots: %d seconds\n", k, tm);
    printf("Memory at end:");
    if (cnt < k) {
        for (int j = 0; j < cnt; j++)
            printf(" %d", frames[j]);
    } else {
        for (int j = 0; j < k; j++)
            printf(" %d", frames[(next + j) % k]);
    }
    printf("\n");

    int ans = -1;
    for (int s = 1; s <= n; s++) {
        int f[MAX];
        int nx = 0, cur_tm = 0;
        for (int j = 0; j < s; j++)
            f[j] = -1;

        for (int i = 0; i < n; i++) {
            int hit = 0;
            for (int j = 0; j < s; j++) {
                if (f[j] == ref[i]) {
                    hit = 1;
                    break;
                }
            }

            if (hit) {
                cur_tm += 1;
            } else {
                cur_tm += 5;
                f[nx] = ref[i];
                nx = (nx + 1) % s;
            }
        }

        if (cur_tm <= t) {
            ans = s;
            break;
        }
    }

    if (ans != -1)
        printf("Minimum slots for time <= %d: %d\n", t, ans);
    else
        printf("Minimum slots for time <= %d: Not possible\n", t);

    return 0;
}
