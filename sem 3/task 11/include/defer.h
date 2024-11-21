#ifndef DEFER_H
#define DEFER_H 1

#include "arena.h"

typedef void (*defer_fn)(void *);
typedef struct {
    defer_fn func;
    void *data;
} lambda_t;

struct DeferSystem {
    struct Arena *allocator;
    struct lambda_t *entries;
};

struct DeferSystem *defer_system_create(struct Arena *allocator);
void defer_system_destroy(struct DeferSystem **system);
void defer_system_init(struct DeferSystem *system, struct Arena *allocator);
void defer_system_deinit(struct DeferSystem *system);

// Data may be NULL if unused
bool defer_system_register(struct DeferSystem *system, lambda_t func);
void defer_system_do_defer(struct DeferSystem *system);

#endif
