#ifndef DEFER_H
#define DEFER_H 1

#include "arena.h"

#ifndef DEFER_SYSTEM_DEFAULT_ENTRIES_CAP
#define DEFER_SYSTEM_DEFAULT_ENTRIES_CAP 1024
#endif

typedef void (*defer_fn)(void *);
typedef struct {
    defer_fn func;
    void *data;
} lambda_t;

struct DeferSystem {
    struct ArenaDynamic *allocator;
    ArenaOffset entries_base;
    size_t entries_at;
    bool own_allocator;
};

// Creates new arena with enough space to hold defer_system and capacity entries
struct DeferSystem *defer_system_create(size_t capacity);
void defer_system_destroy(struct DeferSystem **system);
// Allocator that is passed, it is consumed and owned, thus new allocations outside
// may result in UB. It will not be destroyed or deinit in the end
void defer_system_init(struct DeferSystem *system, struct ArenaDynamic *allocator, size_t capacity);
void defer_system_deinit(struct DeferSystem *system);

// Data may be NULL if unused
void defer_system_register(struct DeferSystem *system, lambda_t func);
void defer_system_do_defer(struct DeferSystem *system);

#endif
