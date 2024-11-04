#include <stdio.h>

int main(void) {
    unsigned long long total = 0;
    int c;
    while ((c = getchar()) != EOF) {
        if ('0' <= c && c <= '9') total += c - '0';
        if ('a' <= c && c <= 'f') total += c - 'a' + 10;
        if ('A' <= c && c <= 'F') total += c - 'A' + 10;
    }
    printf("%llu\n", total);
    return 0;
}
