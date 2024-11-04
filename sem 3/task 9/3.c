#include <stdio.h>

int main(void) {
    int a,b,n;
    scanf("%d%d%d", &a, &b, &n);
    for (int i = 0; i < n; i++) putchar(' ');
    for (int i = a; i < b; i++) {
        if (i == 0) {
            for (int i = 0; i < n; i++) putchar(' ');
            putchar('0');
        } else
            printf(" %*.d", n, i);
    }
    puts("");
    for (int i = a; i < b; i++) {
        if (i == 0) {
            for (int i = 0; i < n - 1; i++) putchar(' ');
            putchar('0');
        } else
            printf("%*.d", n, i);
        for (int j = a; j < b; j++) {
            if ((long long)i * j == 0) {
                for (int k = 0; k < n; k++) putchar(' ');
                putchar('0');
            } else
                printf(" %*.lld", n, (long long)i * j);
        }
        puts("");
    }
    return 0;
}