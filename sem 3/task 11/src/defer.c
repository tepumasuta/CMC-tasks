#include <stdlib.h>
#include <assert.h>

#include "defer.h"

struct DeferSystem *defer_system_create(size_t capacity) {
    struct ArenaDynamic *allocator = arena_dynamic_create(sizeof(lambda_t) * capacity);
    struct DeferSystem *system = malloc(sizeof(*system));
    *system = (struct DeferSystem){
        .allocator = allocator,
        .entries_base = arena_dynamic_alloc(allocator, sizeof(lambda_t) * capacity),
        .entries_at = 0,
        .own_allocator = true,
    };
    return system;
}

void defer_system_destroy(struct DeferSystem **system) {
    assert(system);
    assert(*system);
    if ((*system)->own_allocator) arena_dynamic_destroy(&(*system)->allocator);
    free(*system);
    *system = NULL;
}

void defer_system_init(struct DeferSystem *system, struct ArenaDynamic *allocator, size_t capacity) {
    assert(system);
    bool own = false;
    if (!allocator) {
        allocator = arena_dynamic_create(sizeof(lambda_t) * capacity);
        own = true;
    }
    *system = (struct DeferSystem){
        .allocator = allocator,
        .entries_base = arena_dynamic_alloc(allocator, sizeof(lambda_t) * capacity),
        .entries_at = 0,
        .own_allocator = own,
    };
}

void defer_system_deinit(struct DeferSystem *system) {
    assert(system);
    if (system->own_allocator) arena_dynamic_destroy(&system->allocator);
    *system = (struct DeferSystem){
        .allocator = NULL,
        .entries_base = 0,
        .entries_at = 0,
        .own_allocator = false,
    };
}

// Data may be NULL if unused
void defer_system_register(struct DeferSystem *system, lambda_t func) {
    assert(system);
    *(lambda_t *)arena_dynamic_get_memory(
        system->allocator,
        arena_dynamic_alloc(system->allocator, sizeof(func))
    ) = func;
    system->entries_at++;
}

void defer_system_do_defer(struct DeferSystem *system) {
    assert(system);
    const size_t at = system->entries_at;
    lambda_t *const entries = arena_dynamic_get_memory(system->allocator, system->entries_base);
    for (size_t i = 0; i < at; i++)
        entries[i].func(entries[i].data);
}

