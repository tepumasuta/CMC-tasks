#ifndef _FRACTIONS_H
#define _FRACTIONS_H 1

#include "intdefs.h"
#include <stdbool.h>

struct fraction {
	i64f numerator;
	u64f denominator; 
};

#define FRAC_FROM_INT(_INT) ((struct fraction){ .numerator = (_INT), .denominator = 1 })
#define FRAC_PRINTF "%lu/%lu"
#define FRAC_PRINTF_ARG(_FRAC) (_FRAC).numerator, (_FRAC).denominator

struct fraction fraction_from_int(int value);
struct fraction fraction_make(int num, int den);
double fraction_to_double(struct fraction frac);

struct fraction fraction_add(struct fraction frac1, struct fraction frac2);
struct fraction fraction_sub(struct fraction frac1, struct fraction frac2);
struct fraction fraction_mul(struct fraction frac1, struct fraction frac2);
struct fraction fraction_div(struct fraction frac1, struct fraction frac2);

struct fraction fraction_inv(struct fraction frac);
struct fraction fraction_neg(struct fraction frac);

bool fraction_invalid(struct fraction frac);
bool fraction_equal(struct fraction frac1, struct fraction frac2);
bool fraction_less(struct fraction frac1, struct fraction frac2);
bool fraction_greater(struct fraction frac1, struct fraction frac2);
// Note: Less or equal = not greater
// Note: Greater or equal = not less

#endif // _FRACTIONS_H

