#include <stdio.h>
#include <string.h>

#define MAX 105

int main(void)
{
    int k, n;
    char apps[MAX][35];
    char frames[MAX][35];
    int last[MAX];
    char killed[MAX][35];
    int kcnt = 0, tm = 0;

    scanf("%d", &k);
    scanf("%d", &n);
    for (int i = 0; i < n; i++)
        scanf("%s", apps[i]);

    for (int i = 0; i < k; i++) {
        frames[i][0] = '\0';
        last[i] = -1;
    }

    for (int i = 0; i < n; i++) {
        int hit = 0;
        for (int j = 0; j < k; j++) {
            if (frames[j][0] != '\0' && strcmp(frames[j], apps[i]) == 0) {
                hit = 1;
                last[j] = i;
                break;
            }
        }

        if (hit) {
            tm += 1;
        } else {
            tm += 4;
            int victim = -1;
            for (int j = 0; j < k; j++) {
                if (frames[j][0] == '\0') {
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
                strcpy(killed[kcnt++], frames[victim]);
            }

            strcpy(frames[victim], apps[i]);
            last[victim] = i;
        }
    }

    printf("Total time: %d seconds\n", tm);
    if (kcnt == 0) {
        printf("Apps killed: none\n");
    } else {
        printf("Apps killed:");
        for (int i = 0; i < kcnt; i++)
            printf(" %s", killed[i]);
        printf("\n");
    }

    int ord[MAX], cnt = 0;
    for (int i = 0; i < k; i++) {
        if (frames[i][0] != '\0')
            ord[cnt++] = i;
    }

    for (int i = 0; i < cnt - 1; i++) {
        for (int j = i + 1; j < cnt; j++) {
            if (last[ord[j]] > last[ord[i]]) {
                int tmp = ord[i];
                ord[i] = ord[j];
                ord[j] = tmp;
            }
        }
    }

    printf("Recent apps:");
    for (int i = 0; i < cnt; i++)
        printf(" %s", frames[ord[i]]);
    printf("\n");

    return 0;
}
