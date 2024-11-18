#include "arena.h"

#include <stdlib.h>
#include <assert.h>

struct Arena *arena_create(size_t initial_capacity) {
    struct Arena *arena = malloc(sizeof(*arena) + sizeof(char) * initial_capacity);
    if (!arena) return arena;
    *arena = (struct Arena){
        .memory = arena + 1,
        .alloced = true,
        .at = 0,
        .cap = initial_capacity,
    };
    return arena;
}

struct Arena *arena_create_static_from_buffer(void *buffer, size_t buffer_capacity) {
    struct Arena *arena = malloc(sizeof(*arena));
    if (!arena) return arena;
    *arena = (struct Arena){
        .memory = buffer,
        .alloced = false,
        .at = 0,
        .cap = buffer_capacity,
    };
    return arena;
}

void arena_destroy(struct Arena **arena) {
    assert(arena);
    assert(*arena);
    if ((*arena)->alloced)
        free(*arena);
    *arena = NULL;
}

void arena_init(struct Arena *arena, size_t initial_capacity) {
    assert(arena);
    void *mem = malloc(sizeof(char) * initial_capacity);
    *arena = (struct Arena){
        .memory = mem,
        .alloced = mem ? true : false,
        .at = 0,
        .cap = mem ? initial_capacity : 0,
    };
}

void arena_deinit(struct Arena *arena) {
    assert(arena);
    if (arena->alloced) free(arena->memory);
    *arena = (struct Arena){
        .memory = NULL,
        .alloced = false,
        .at = 0,
        .cap = 0,
    };
}

void arena_init_static_from_buffer(struct Arena *arena, void *buffer, size_t buffer_capacity) {
    assert(arena);
    *arena = (struct Arena){
        .memory = buffer,
        .alloced = false,
        .at = 0,
        .cap = buffer_capacity,
    };
}
