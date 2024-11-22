#include <stdlib.h>
#include <assert.h>

#include "defer.h"

struct DeferSystem *defer_system_create() {
    struct ArenaDynamic *allocator = arena_dynamic_create(
        sizeof(lambda_t) * DEFER_SYSTEM_DEFAULT_INITIAL_ENTRIES_CAP
    );
    struct DeferSystem *system = malloc(sizeof(*system));
    *system = (struct DeferSystem){
        .allocator = allocator,
        .entries_base = arena_dynamic_at(allocator),
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

void defer_system_init(struct DeferSystem *system, struct ArenaDynamic *allocator) {
    assert(system);
    bool own = false;
    if (!allocator) {
        allocator = arena_dynamic_create(
            sizeof(lambda_t) * DEFER_SYSTEM_DEFAULT_INITIAL_ENTRIES_CAP
        );
        own = true;
    }
    *system = (struct DeferSystem){
        .allocator = allocator,
        .entries_base = arena_dynamic_at(allocator),
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
    assert(func.func);
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

struct DeferSystem *defer_system_try_create() {
    struct ArenaDynamic *allocator = arena_dynamic_try_create(
        sizeof(lambda_t) * DEFER_SYSTEM_DEFAULT_INITIAL_ENTRIES_CAP
    );
    if (!allocator) return NULL;
    struct DeferSystem *system = malloc(sizeof(*system));
    if (!system) {
        arena_dynamic_destroy(&allocator);
        return NULL;
    }
    *system = (struct DeferSystem){
        .allocator = allocator,
        .entries_base = arena_dynamic_at(allocator),
        .entries_at = 0,
        .own_allocator = true,
    };
    return system;
}

bool defer_system_try_destroy(struct DeferSystem **system) {
    assert(system);
    if (!*system) return false;
    if ((*system)->own_allocator && !arena_dynamic_try_destroy(&(*system)->allocator)) return false;
    free(*system);
    *system = NULL;
    return true;
}

bool defer_system_try_init(struct DeferSystem *system, struct ArenaDynamic *allocator) {
    assert(system);
    bool own = false;
    if (!allocator) {
        allocator = arena_dynamic_create(DEFER_SYSTEM_DEFAULT_INITIAL_ENTRIES_CAP);
        if (!allocator) return false;
    }
    own = true;
    *system = (struct DeferSystem){
        .allocator = allocator,
        .entries_base = arena_dynamic_at(allocator),
        .entries_at = 0,
        .own_allocator = own,
    };
    return true;
}

bool defer_system_try_deinit(struct DeferSystem *system) {
    assert(system);
    if (system->own_allocator && !arena_dynamic_try_destroy(&system->allocator)) return false;
    return true;
}

// Data may be NULL if unused
bool defer_system_try_register(struct DeferSystem *system, lambda_t func) {
    assert(system);
    assert(func.func);
    ArenaOffset new_loc;
    if (!arena_dynamic_try_alloc(system->allocator, sizeof(func), &new_loc)) return false;
    *(lambda_t *)arena_dynamic_get_memory(
        system->allocator,
        new_loc
    ) = func;
    system->entries_at++;
    return true;
}
