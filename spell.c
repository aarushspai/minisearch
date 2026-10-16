#include <string.h>
#include "common.h"
#include "spell.h"

static int min3(int a, int b, int c) {
    int m = a < b ? a : b;
    return m < c ? m : c;
}

/* dp[i][j] = distance between the first i letters of a and the first j letters of b.
   Each cell is built from three neighbours: delete, insert, or replace/keep. */
int edit_distance(const char *a, const char *b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    if (la >= WORD_LEN || lb >= WORD_LEN) return 99;
    int dp[WORD_LEN][WORD_LEN];
    for (int i = 0; i <= la; i++) dp[i][0] = i;       /* turning a prefix into "" = delete all */
    for (int j = 0; j <= lb; j++) dp[0][j] = j;       /* turning "" into a prefix = insert all */
    for (int i = 1; i <= la; i++) {
        for (int j = 1; j <= lb; j++) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = min3(dp[i - 1][j] + 1,          /* delete  */
                            dp[i][j - 1] + 1,          /* insert  */
                            dp[i - 1][j - 1] + cost);  /* replace or keep */
        }
    }
    return dp[la][lb];
}
