#include <stdio.h>
#include <stdbool.h>


bool is_perfect(unsigned number) {
	unsigned total = 0;
	for (unsigned div = 1; div * 2 <= number; div++)
		total += number % div == 0 ? div : 0;
	
	return number == total;
}

int main(void) {
	unsigned plank;
	if (scanf("%u", &plank) != 1)
		return -1;

	for (unsigned num = 1; num < plank; num++)
		if (is_perfect(num))
			printf("%d ", num);
	putc('\n', stdout); 

	return 0;
}

