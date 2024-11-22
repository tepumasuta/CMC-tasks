#include <stdio.h>
#include <stdlib.h>

#include "arena.h"
#include "defer.h"

struct DeferSystem defer_system;

void cleanup(void) {
    defer_system_do_defer(&defer_system);
    defer_system_deinit(&defer_system);
}

int main(void) {
    if (!defer_system_try_init(&defer_system, NULL)) abort();
    if (atexit(cleanup)) abort();

    return 0;
}
