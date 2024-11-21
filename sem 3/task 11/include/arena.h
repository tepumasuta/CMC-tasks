#ifndef _ARENA_H
#define _ARENA_H 1

#include <stddef.h>
#include <stdbool.h>

struct Arena {
    void *memory;
    size_t cap, at;
    bool alloced;
};

/* Heap memory allocation */
struct Arena *arena_create(size_t initial_capacity);
struct Arena *arena_create_static_from_buffer(void *buffer, size_t buffer_capacity);
void arena_destroy(struct Arena **arena);

/* In-place memory allocation */
void arena_init(struct Arena *arena, size_t initial_capacity);
// Doesn't have to be deallocated
void arena_init_static_from_buffer(struct Arena *arena, void *buffer, size_t buffer_capacity);
void arena_deinit(struct Arena *arena);


/* Fast methods without checks */
void *arena_alloc(struct Arena *arena, size_t size);
// Assumes arena alloced
void arena_reset(struct Arena *arena);

/* Info methods */
size_t arena_capacity(struct Arena *arena);
size_t arena_allocated(struct Arena *arena);
// Whether was created from buffer
bool arena_is_static(struct Arena *arena);  

/* Safe methods with checks */
struct Arena *arena_try_create(size_t initial_capacity);
struct Arena *arena_try_create_static_from_buffer(void *buffer, size_t buffer_capacity);
bool arena_try_destroy(struct Arena **arena);
bool arena_try_init(struct Arena *arena, size_t initial_capacity);
bool arena_try_init_static_from_buffer(struct Arena *arena, void *buffer, size_t buffer_capacity);
bool arena_try_deinit(struct Arena *arena);

void *arena_try_alloc(struct Arena *arena, size_t size);

#endif
