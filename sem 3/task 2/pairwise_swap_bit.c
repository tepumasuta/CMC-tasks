#include <stdio.h>
#include <ctype.h>

#define BASE_TYPE char

typedef BASE_TYPE signed_t;
typedef unsigned BASE_TYPE unsigned_t;

#define BYTE_MASK 0x55

signed_t swap_bit(signed_t value) {
    // Gen mask
    unsigned_t mask = 0;
    for (register char i = 0; i < sizeof(signed_t); i++)
        mask = (mask << 8) | BYTE_MASK;
    
    unsigned_t even = value & mask, odd = value & (mask << 1);
    return (even << 1) | (odd >> 1);
}

void print_bits(unsigned_t num) {
    unsigned_t reversed = 0;
    for (size_t i = 0; i < sizeof(unsigned_t) * 8; i++) {
        reversed = (reversed << 1) + (num & 1);
        num >>= 1;
    }
    for (size_t i = 0; i < sizeof(unsigned_t) * 8; i++) {
        putc((reversed & 1) + '0', stdout);
        reversed >>= 1;
    }
    puts("");
}

signed_t read_num(void) {
    long long result = -1;
    scanf("%lld", &result);
    return result;
}

int main(void) {
    signed_t num = read_num();
    print_bits(num);
    num = swap_bit(num);
    print_bits(num);

    return 0;
}
