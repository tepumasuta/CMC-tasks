#include "stack.h"

#ifndef STACK_SIZE
#define STACK_SIZE 5
#endif

static int stack[STACK_SIZE];
static unsigned sp;

bool stack_try_push(int sym) {
    if (sp == STACK_SIZE) return false;
    stack[sp++] = sym;
    return true;
}
bool stack_try_pop(int *sym) {
    if (!sp) return false;
    *sym = stack[--sp];
    return true;
}
bool stack_try_peek(int *sym) {
    if (!sp) return false;
    *sym = stack[sp - 1];
    return true;
}

void stack_push(int sym) { stack[sp++] = sym; }
int stack_pop() { return stack[--sp]; }
int stack_peek() { return stack[sp - 1]; }

bool stack_empty() { return !sp; }
void stack_reset() { sp = 0; }
unsigned stack_size() { return sp; }
unsigned stack_cap() { return STACK_SIZE; }
