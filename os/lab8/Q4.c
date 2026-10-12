#include <stdio.h>
#include <string.h>

#define MAX 105

int main(void)
{
    int c, s;
    char name[MAX][35];
    int sz[MAX];

    scanf("%d", &c);
    scanf("%d", &s);
    for (int i = 0; i < s; i++)
        scanf("%s %d", name[i], &sz[i]);

    int p;
    scanf("%d", &p);
    char play[MAX][35];
    for (int i = 0; i < p; i++)
        scanf("%s", play[i]);

    char lst[MAX][35];
    int ltim[MAX];
    int lcnt = 0, lused = 0, lhits = 0, lmiss = 0, ldown = 0;

    for (int t = 0; t < p; t++) {
        int msz = 0;
        for (int i = 0; i < s; i++) {
            if (strcmp(name[i], play[t]) == 0) {
                msz = sz[i];
                break;
            }
        }

        int hit = 0, idx = -1;
        for (int j = 0; j < lcnt; j++) {
            if (strcmp(lst[j], play[t]) == 0) {
                hit = 1;
                idx = j;
                break;
            }
        }

        if (hit) {
            lhits++;
            ltim[idx] = t;
        } else {
            lmiss++;
            if (msz > c)
                continue;

            while (lused + msz > c) {
                int victim = 0;
                for (int j = 1; j < lcnt; j++) {
                    if (ltim[j] < ltim[victim])
                        victim = j;
                }
                int vsz = 0;
                for (int i = 0; i < s; i++) {
                    if (strcmp(name[i], lst[victim]) == 0) {
                        vsz = sz[i];
                        break;
                    }
                }
                lused -= vsz;
                for (int j = victim; j < lcnt - 1; j++) {
                    strcpy(lst[j], lst[j + 1]);
                    ltim[j] = ltim[j + 1];
                }
                lcnt--;
            }

            strcpy(lst[lcnt], play[t]);
            ltim[lcnt] = t;
            lcnt++;
            lused += msz;
            ldown += msz;
        }
    }

    char fst[MAX][35];
    int ftim[MAX];
    int fcnt = 0, fused = 0, fhits = 0, fmiss = 0, fdown = 0;

    for (int t = 0; t < p; t++) {
        int msz = 0;
        for (int i = 0; i < s; i++) {
            if (strcmp(name[i], play[t]) == 0) {
                msz = sz[i];
                break;
            }
        }

        int hit = 0;
        for (int j = 0; j < fcnt; j++) {
            if (strcmp(fst[j], play[t]) == 0) {
                hit = 1;
                break;
            }
        }

        if (hit) {
            fhits++;
        } else {
            fmiss++;
            if (msz > c)
                continue;

            while (fused + msz > c) {
                int victim = 0;
                for (int j = 1; j < fcnt; j++) {
                    if (ftim[j] < ftim[victim])
                        victim = j;
                }
                int vsz = 0;
                for (int i = 0; i < s; i++) {
                    if (strcmp(name[i], fst[victim]) == 0) {
                        vsz = sz[i];
                        break;
                    }
                }
                fused -= vsz;
                for (int j = victim; j < fcnt - 1; j++) {
                    strcpy(fst[j], fst[j + 1]);
                    ftim[j] = ftim[j + 1];
                }
                fcnt--;
            }

            strcpy(fst[fcnt], play[t]);
            ftim[fcnt] = t;
            fcnt++;
            fused += msz;
            fdown += msz;
        }
    }

    int lord[MAX];
    for (int i = 0; i < lcnt; i++)
        lord[i] = i;
    for (int i = 0; i < lcnt - 1; i++) {
        for (int j = i + 1; j < lcnt; j++) {
            if (ltim[lord[j]] < ltim[lord[i]]) {
                int tmp = lord[i];
                lord[i] = lord[j];
                lord[j] = tmp;
            }
        }
    }

    int ford[MAX];
    for (int i = 0; i < fcnt; i++)
        ford[i] = i;
    for (int i = 0; i < fcnt - 1; i++) {
        for (int j = i + 1; j < fcnt; j++) {
            if (ftim[ford[j]] < ftim[ford[i]]) {
                int tmp = ford[i];
                ford[i] = ford[j];
                ford[j] = tmp;
            }
        }
    }

    printf("LRU: hits %d, misses %d, downloaded %d MB, stored:", lhits, lmiss, ldown);
    for (int i = 0; i < lcnt; i++)
        printf(" %s", lst[lord[i]]);
    printf("\n");

    printf("FIFO: hits %d, misses %d, downloaded %d MB, stored:", fhits, fmiss, fdown);
    for (int i = 0; i < fcnt; i++)
        printf(" %s", fst[ford[i]]);
    printf("\n");

    if (ldown < fdown)
        printf("Better policy: LRU\n");
    else if (fdown < ldown)
        printf("Better policy: FIFO\n");
    else
        printf("Better policy: Both equal\n");

    return 0;
}
