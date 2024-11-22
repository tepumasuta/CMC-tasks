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

// Look-up index into arena relative to its base
typedef ptrdiff_t ArenaOffset;
// Expandable pointer unreliable arena
struct ArenaDynamic {
    void *memory;
    size_t cap, at;
};

/* Heap memory allocation */
struct ArenaStatic *arena_static_create(size_t capacity);
struct ArenaStatic *arena_static_create_static_from_buffer(void *buffer, size_t buffer_capacity);
void arena_static_destroy(struct ArenaStatic **arena);
struct ArenaDynamic *arena_dynamic_create(size_t initial_capacity);
void arena_dynamic_destroy(struct ArenaDynamic **arena);

/* In-place memory allocation */
void arena_static_init(struct ArenaStatic *arena, size_t capacity);
// Doesn't have to be deallocated
void arena_static_init_static_from_buffer(struct ArenaStatic *arena, void *buffer, size_t buffer_capacity);
void arena_static_deinit(struct ArenaStatic *arena);
void arena_dynamic_init(struct ArenaDynamic *arena, size_t initial_capacity);
void arena_dynamic_deinit(struct ArenaDynamic *arena);


/* Fast methods without checks */
void *arena_static_alloc(struct ArenaStatic *arena, size_t size);
void arena_static_reset(struct ArenaStatic *arena);
ArenaOffset arena_dynamic_alloc(struct ArenaDynamic *arena, size_t size);
void *arena_dynamic_get_memory(struct ArenaDynamic *arena, ArenaOffset offset);
void arena_dynamic_reserve(struct ArenaDynamic *arena, size_t capacity);
void arena_dynamic_reset(struct ArenaDynamic *arena);

/* Info methods */
size_t arena_static_capacity(struct ArenaStatic *arena);
size_t arena_static_allocated(struct ArenaStatic *arena);
bool arena_static_is_from_buffer(struct ArenaStatic *arena);  
size_t arena_dynamic_capacity(struct ArenaDynamic *arena);
size_t arena_dynamic_allocated(struct ArenaDynamic *arena);
ArenaOffset arena_dynamic_at(struct ArenaDynamic *arena);

/* Safe methods with checks */
struct ArenaStatic *arena_static_try_create(size_t capacity);
struct ArenaStatic *arena_static_try_create_static_from_buffer(void *buffer, size_t buffer_capacity);
bool arena_static_try_destroy(struct ArenaStatic **arena);
bool arena_static_try_init(struct ArenaStatic *arena, size_t initial_capacity);
bool arena_static_try_init_static_from_buffer(struct ArenaStatic *arena, void *buffer, size_t buffer_capacity);
bool arena_static_try_deinit(struct ArenaStatic *arena);
struct ArenaDynamic *arena_dynamic_try_create(size_t initial_capacity);
bool arena_dynamic_try_destroy(struct ArenaDynamic **arena);
bool arena_dynamic_try_init(struct ArenaDynamic *arena, size_t initial_capacity);
bool arena_dynamic_try_deinit(struct ArenaDynamic *arena);

void *arena_static_try_alloc(struct ArenaStatic *arena, size_t size);
bool arena_dynamic_try_alloc(struct ArenaDynamic *arena, size_t size, ArenaOffset *offset);
void *arena_dynamic_try_get_memory(struct ArenaDynamic *arena, ArenaOffset offset);
bool arena_dynamic_try_reserve(struct ArenaDynamic *arena, size_t capacity);

#endif
