#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"
#include <limits.h>

#define IsNum(a) ((a) >= '0' && (a) <= '9')
#define IsSign(a) ((a) == '+' || (a) == '-' || (a) == '*' || (a) == '/' || (a) == '%')
#define SequenceErr do {fprintf(stderr, "Error: Invalid sequence\n"); exit(1);} while(0)
#define IntOverflowErr do {fprintf(stderr, "Warning: Integer overflow\n");} while(0)
#define DivByZeroErr do {fprintf(stderr, "Error: Division by zero\n"); exit(1);} while(0)
#define StackOverflowErr do {fprintf(stderr, "Error: Stack overflow\n"); exit(1);} while(0)

int main(void){
	char s;
	int num = 0, tmp;
	int fir, sec;
	bool sec_g, fir_g;
	bool space_ind = true;
	while((s = getchar()) != EOF && s != '\n'){
		if(IsNum(s)){
			tmp = num;
			num = num * 10 + (s - '0');
			if (tmp > num)
				IntOverflowErr;
			space_ind = false;
		}
		else if(IsSign(s)){
			sec_g = stack_try_pop(&sec);
			fir_g = stack_try_pop(&fir);
			if(!sec_g || !fir_g)
				SequenceErr;
			switch (s){
				case '+':
					num = fir + sec;
					if ((sec > 0 && fir > INT_MAX - sec) || (sec < 0 && fir < INT_MIN - sec))
						IntOverflowErr;
					break;
				case '-':
					num = fir - sec;
					if ((sec > 0 && fir < INT_MIN + sec) || (sec < 0 && fir > INT_MAX + sec))
						IntOverflowErr;
					break;
				case '*':
					num = fir * sec;
					if (num == INT_MIN && sec == -1)
						IntOverflowErr;
					else if (sec != 0 && num / sec != fir)
						IntOverflowErr;
					break;
				case '/':
					if(!sec)
						DivByZeroErr;
					if (fir == INT_MIN && sec == -1) {
						IntOverflowErr;
						num = INT_MIN;
					} else
						num = fir / sec;
					break;
				case '%':
					if(!sec)
						DivByZeroErr;
					num = fir % sec;
			}
			space_ind = false;
		}
		else if(s == ' '){
			if(space_ind)
				continue;
			if(!stack_try_push(num))
				StackOverflowErr;
			num = 0;
			space_ind = true;
		}
		else
			SequenceErr;
	}
	if(!space_ind && !stack_try_push(num)){
		StackOverflowErr;
	}
	if(!stack_try_pop(&num))
		SequenceErr;
	if(!stack_empty()){
		SequenceErr;
	}
	printf("%d\n", num);
	return 0;
}
