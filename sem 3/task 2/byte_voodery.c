#include <stdio.h>
#include <ctype.h>

#define BYTE_MASK 0xFF

unsigned dark_byte_voodery_enby(unsigned num, int n1, int n2) {
    n1 = sizeof(n1) - n1;
    n2 = sizeof(n2) - n2;
    unsigned m1 = BYTE_MASK << n1 * 8, m2 = BYTE_MASK << n2 * 8;
    return (num & m1) >> n1 * 8 == (num & m2) >> n2 * 8
         ? (m1 | m2) & num
         : (m1 | m2) ^ num;
}

void print_bits(unsigned num) {
    unsigned reversed = 0;
    for (size_t i = 0; i < sizeof(unsigned) * 8; i++) {
        reversed = (reversed << 1) + (num & 1);
        num >>= 1;
    }
    for (size_t i = 0; i < sizeof(unsigned) * 8; i++) {
        putc((reversed & 1) + '0', stdout);
        if (i % 8 == 7 && i != sizeof(unsigned) * 8 - 1) putc('\'', stdout);
        reversed >>= 1;
    }
    puts("");
}

int main(void) {
    unsigned num, p1, p2;
    scanf("%d%d%d", &num, &p1, &p2);
    print_bits(num);
    num = dark_byte_voodery_enby(num, p1, p2);
    print_bits(num);

    return 0;
}
