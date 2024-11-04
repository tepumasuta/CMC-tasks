#include <stdio.h>
#include <stdbool.h>

bool is_prime(unsigned number) {
	if (number <= 1) return false;
	if (number % 2 == 0) return number == 2;
	for (unsigned div = 3; div * div <= number; div += 2)
		if (number % div == 0)
			return false;
	return true;
}

int main(void) {
	unsigned plank;
	if (scanf("%u", &plank) != 1)
		return -1;

	for (unsigned x = 3; x + 2 < plank; x++)
		if (is_prime(x) && is_prime(x + 2))
			printf("(%u,%u) ", x, x + 2);
	putc('\n', stdout);

	return 0;
}

