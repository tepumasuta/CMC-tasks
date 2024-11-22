#include "arena.h"

#include <stdlib.h>
#include <assert.h>

struct ArenaStatic *arena_static_create(size_t capacity) {
    struct ArenaStatic *arena = malloc(sizeof(*arena) + sizeof(char) * capacity);
    *arena = (struct ArenaStatic){
        .memory = arena + 1,
        .alloced = true,
        .at = 0,
        .cap = capacity,
    };
    return arena;
}

struct ArenaStatic *arena_static_create_static_from_buffer(void *buffer, size_t buffer_capacity) {
    struct ArenaStatic *arena = malloc(sizeof(*arena));
    *arena = (struct ArenaStatic){
        .memory = buffer,
        .alloced = false,
        .at = 0,
        .cap = buffer_capacity,
    };
    return arena;
}

void arena_static_destroy(struct ArenaStatic **arena) {
    assert(arena);
    assert(*arena);
    free(*arena);
    *arena = NULL;
}

void arena_static_init(struct ArenaStatic *arena, size_t capacity) {
    assert(arena);
    void *mem = malloc(sizeof(char) * capacity);
    *arena = (struct ArenaStatic){
        .memory = mem,
        .alloced = true,
        .at = 0,
        .cap = capacity,
    };
}

void arena_static_deinit(struct ArenaStatic *arena) {
    assert(arena);
    if (arena->alloced) free(arena->memory);
    *arena = (struct ArenaStatic){
        .memory = NULL,
        .alloced = false,
        .at = 0,
        .cap = 0,
    };
}

void arena_static_init_static_from_buffer(struct ArenaStatic *arena, void *buffer, size_t buffer_capacity) {
    assert(arena);
    *arena = (struct ArenaStatic){
        .memory = buffer,
        .alloced = false,
        .at = 0,
        .cap = buffer_capacity,
    };
}

void *arena_static_alloc(struct ArenaStatic *arena, size_t size) {
    assert(arena);
    void *mem = (char *)arena->memory + arena->at;
    arena->at += size;
    return mem;
}

void arena_static_reset(struct ArenaStatic *arena) {
    assert(arena);
    arena->at = 0;
}

size_t arena_static_capacity(struct ArenaStatic *arena) {
    assert(arena);
    return arena->cap;
}

size_t arena_static_allocated(struct ArenaStatic *arena) {
    assert(arena);
    return arena->at;
}

bool arena_static_is_from_buffer(struct ArenaStatic *arena) {
    assert(arena);
    return !arena->alloced;
}

struct ArenaStatic *arena_static_try_create(size_t capacity) {
    struct ArenaStatic *arena = malloc(sizeof(*arena) + sizeof(char) * capacity);
    if (!arena) return NULL;

    *arena = (struct ArenaStatic){
        .memory = arena + 1,
        .alloced = true,
        .at = 0,
        .cap = capacity,
    };
    return arena;
}

struct ArenaStatic *arena_static_try_create_static_from_buffer(void *buffer, size_t buffer_capacity) {
    struct ArenaStatic *arena = malloc(sizeof(*arena));
    if (!arena) return NULL;

    *arena = (struct ArenaStatic){
        .memory = buffer,
        .alloced = false,
        .at = 0,
        .cap = buffer_capacity,
    };
    return arena;
}

bool arena_static_try_destroy(struct ArenaStatic **arena) {
    assert(arena);
    if (!*arena) return false;
    if ((*arena)->alloced) free(*arena);
    *arena = NULL;
    return true;
}

bool arena_static_try_init(struct ArenaStatic *arena, size_t capacity) {
    assert(arena);
    void *mem = malloc(sizeof(char) * capacity);
    if (!mem) return false;
    *arena = (struct ArenaStatic){
        .memory = mem,
        .alloced = true,
        .at = 0,
        .cap = capacity,
    };
    return true;
}

bool arena_static_try_init_static_from_buffer(struct ArenaStatic *arena, void *buffer, size_t buffer_capacity) {
    assert(arena);
    if (!buffer) return false;
    *arena = (struct ArenaStatic){
        .memory = buffer,
        .alloced = false,
        .at = 0,
        .cap = buffer_capacity,
    };
    return true;
}

bool arena_static_try_deinit(struct ArenaStatic *arena) {
    assert(arena);
    bool success = arena->alloced;
    if (success) free(arena->memory);
    *arena = (struct ArenaStatic){
        .memory = NULL,
        .alloced = false,
        .at = 0,
        .cap = 0,
    };
    return success;
}

void *arena_static_try_alloc(struct ArenaStatic *arena, size_t size) {
    assert(arena);
    if (arena->alloced && arena->cap < arena->at + size) return NULL;
    void *mem = (char *)arena->memory + arena->at;
    arena->at += size;
    return mem;
}
