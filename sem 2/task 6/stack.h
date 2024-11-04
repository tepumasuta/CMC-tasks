#pragma once

#include <stdbool.h>

bool stack_try_push(int sym);
bool stack_try_pop(int *sym);
bool stack_try_peek(int *sym);

void stack_push(int sym);
int stack_pop();
int stack_peek();

bool stack_empty();
void stack_reset();
unsigned stack_size();
unsigned stack_cap();

