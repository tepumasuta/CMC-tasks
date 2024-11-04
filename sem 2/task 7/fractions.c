#include "fractions.h"

static u64f gcd(u64f x, u64f y) {
	while (y) {
		u64f tmp = y;
		y = x % y;
		x = tmp;
	}
	return x;
}

static inline u64f absi64f(i64f x) {
	return x < 0 ? -x : x;
}

struct fraction fraction_from_int(int value) {
	return (struct fraction){
		.numerator = value,
		.denominator = 1ULL,
	};
}

struct fraction fraction_make(int num, int den) {
	if (!den) return (struct fraction){ 0 };
	if (den < 0) {
		num = -num;
		den = -den;
	}

	u64f div = gcd(absi64f(num), den);

	return (struct fraction){
		.numerator = num / (i64f)div,
		.denominator = den / div,
	};
}

double fraction_to_double(struct fraction frac) {
	return (double)frac.numerator / frac.denominator;
}

struct fraction fraction_add(struct fraction frac1, struct fraction frac2) {
	u64f den = frac1.denominator * frac2.denominator;
	if (!den) return (struct fraction){ 0 };
	i64f num = frac1.numerator * (i64f)frac2.denominator + frac2.numerator * (i64f)frac1.denominator;
	u64f div = gcd(absi64f(num), den);

	return (struct fraction){
		.numerator = num / (i64f)div,
		.denominator = den / div,
	};
}

struct fraction fraction_sub(struct fraction frac1, struct fraction frac2) {
	u64f den = frac1.denominator * frac2.denominator;
	if (!den) return (struct fraction){ 0 };
	i64f num = frac1.numerator * (i64f)frac2.denominator - frac2.numerator * (i64f)frac1.denominator;
	u64f div = gcd(absi64f(num), den);

	return (struct fraction){
		.numerator = num / (i64f)div,
		.denominator = den / div,
	};
}

struct fraction fraction_mul(struct fraction frac1, struct fraction frac2) {
	u64f den = frac1.denominator * frac2.denominator;
	if (!den) return (struct fraction){ 0 };
	i64f num = frac1.numerator * frac2.numerator;
	u64f div = gcd(absi64f(num), den);

	return (struct fraction){
		.numerator = num / (i64f)div,
		.denominator = den / div,
	};
}

struct fraction fraction_div(struct fraction frac1, struct fraction frac2) {
	i64f den = (i64f)frac1.denominator * frac2.numerator;
	if (!den) return (struct fraction){ 0 };
	i64f num = frac1.numerator * (i64f)frac2.denominator;
	if (den < 0) {
		den = -den;
		num = -num;
	}
	u64f div = gcd(absi64f(num), den);

	return (struct fraction){
		.numerator = num / (i64f)div,
		.denominator = den / div,
	};
}

struct fraction fraction_inv(struct fraction frac) {
	if (!frac.numerator) return (struct fraction){ 0 };
	return (struct fraction){
		.numerator = frac.denominator,
		.denominator = frac.numerator,
	};
}

struct fraction fraction_neg(struct fraction frac) {
	return (struct fraction){
		.numerator = -frac.numerator,
		.denominator = frac.denominator,
	};
}

bool fraction_invalid(struct fraction frac) {
	return !frac.numerator && !frac.denominator;
}

bool fraction_equal(struct fraction frac1, struct fraction frac2) {
	return frac1.numerator == frac2.numerator && frac1.denominator == frac2.denominator;
}

bool fraction_less(struct fraction frac1, struct fraction frac2) {
	return frac1.numerator * (i64f)frac2.denominator < frac2.numerator * (i64f)frac1.denominator;
}

bool fraction_greater(struct fraction frac1, struct fraction frac2) {
	return frac1.numerator * (i64f)frac2.denominator > frac2.numerator * (i64f)frac1.denominator;
}

