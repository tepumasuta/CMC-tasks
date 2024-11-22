#ifndef _ARENA_H
#define _ARENA_H 1

#include <stddef.h>
#include <stdbool.h>

// Non-expandable pointer reliable arena
struct ArenaStatic {
    void *memory;
    size_t cap, at;
    bool alloced;
};

/* Heap memory allocation */
struct ArenaStatic *arena_static_create(size_t capacity);
struct ArenaStatic *arena_static_create_static_from_buffer(void *buffer, size_t buffer_capacity);
void arena_static_destroy(struct ArenaStatic **arena);

/* In-place memory allocation */
void arena_static_init(struct ArenaStatic *arena, size_t capacity);
// Doesn't have to be deallocated
void arena_static_init_static_from_buffer(struct ArenaStatic *arena, void *buffer, size_t buffer_capacity);
void arena_static_deinit(struct ArenaStatic *arena);


/* Fast methods without checks */
void *arena_static_alloc(struct ArenaStatic *arena, size_t size);
// Assumes arena alloced
void arena_static_reset(struct ArenaStatic *arena);

/* Info methods */
size_t arena_static_capacity(struct ArenaStatic *arena);
size_t arena_static_allocated(struct ArenaStatic *arena);
// Whether was created from buffer
bool arena_static_is_from_buffer(struct ArenaStatic *arena);  

/* Safe methods with checks */
struct ArenaStatic *arena_static_try_create(size_t capacity);
struct ArenaStatic *arena_static_try_create_static_from_buffer(void *buffer, size_t buffer_capacity);
bool arena_static_try_destroy(struct ArenaStatic **arena);
bool arena_static_try_init(struct ArenaStatic *arena, size_t capacity);
bool arena_static_try_init_static_from_buffer(struct ArenaStatic *arena, void *buffer, size_t buffer_capacity);
bool arena_static_try_deinit(struct ArenaStatic *arena);

void *arena_static_try_alloc(struct ArenaStatic *arena, size_t size);

#endif
