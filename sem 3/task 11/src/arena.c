#include "arena.h"

#include <stdlib.h>
#include <assert.h>

struct Arena *arena_create(size_t initial_capacity) {
    struct Arena *arena = malloc(sizeof(*arena) + sizeof(char) * initial_capacity);
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
    free(*arena);
    *arena = NULL;
}

void arena_init(struct Arena *arena, size_t initial_capacity) {
    assert(arena);
    void *mem = malloc(sizeof(char) * initial_capacity);
    *arena = (struct Arena){
        .memory = mem,
        .alloced = true,
        .at = 0,
        .cap = initial_capacity,
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

void *arena_alloc(struct Arena *arena, size_t size) {
    assert(arena);
    void *mem = (char *)arena->memory + arena->at;
    arena->at += size;
    return mem;
}

void arena_reset(struct Arena *arena) {
    assert(arena);
    arena->at = 0;
}

size_t arena_capacity(struct Arena *arena) {
    assert(arena);
    return arena->cap;
}

size_t arena_allocated(struct Arena *arena) {
    assert(arena);
    return arena->at;
}

bool arena_is_static(struct Arena *arena) {
    assert(arena);
    return !arena->alloced;
}

struct Arena *arena_try_create(size_t initial_capacity) {
    struct Arena *arena = malloc(sizeof(*arena) + sizeof(char) * initial_capacity);
    if (!arena) return NULL;

    *arena = (struct Arena){
        .memory = arena + 1,
        .alloced = true,
        .at = 0,
        .cap = initial_capacity,
    };
    return arena;
}

struct Arena *arena_try_create_static_from_buffer(void *buffer, size_t buffer_capacity) {
    struct Arena *arena = malloc(sizeof(*arena));
    if (!arena) return NULL;

    *arena = (struct Arena){
        .memory = buffer,
        .alloced = false,
        .at = 0,
        .cap = buffer_capacity,
    };
    return arena;
}

bool arena_try_destroy(struct Arena **arena) {
    assert(arena);
    assert(*arena);
    bool success = (*arena)->alloced;
    if (success) free(*arena);
    *arena = NULL;
    return success;

}

bool arena_try_init(struct Arena *arena, size_t initial_capacity) {
    assert(arena);
    void *mem = malloc(sizeof(char) * initial_capacity);
    if (!mem) return false;
    *arena = (struct Arena){
        .memory = mem,
        .alloced = true,
        .at = 0,
        .cap = initial_capacity,
    };
    return true;
   
}

bool arena_try_init_static_from_buffer(struct Arena *arena, void *buffer, size_t buffer_capacity) {
    assert(arena);
    if (!buffer) return false;
    *arena = (struct Arena){
        .memory = buffer,
        .alloced = false,
        .at = 0,
        .cap = buffer_capacity,
    };
    return true;
}

bool arena_try_deinit(struct Arena *arena) {
    assert(arena);
    if (arena->alloced) free(arena->memory);
    *arena = (struct Arena){
        .memory = NULL,
        .alloced = false,
        .at = 0,
        .cap = 0,
    };
    return true;
}

void *arena_try_alloc(struct Arena *arena, size_t size) {
    assert(arena);
    if (arena->alloced && arena->cap < arena->at + size) return NULL;
    void *mem = (char *)arena->memory + arena->at;
    arena->at += size;
    return mem;
}
