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

void *arena_alloc(struct Arena *arena, size_t size) {
    assert(arena);
    if (arena->alloced && arena->cap < arena->at + size) {
        size_t new_cap = arena->cap;
        if (!new_cap) new_cap = 1;
        while (new_cap < arena->at + size) new_cap *= 2;
        void *new = realloc(arena->memory, new_cap);
        arena->cap = new_cap;
        arena->memory = new;
    }
    void *mem = (char *)arena->memory + arena->at;
    arena->at += size;
    return mem;
}

void arena_reserve(struct Arena *arena, size_t capacity) {
    assert(arena);
    void *new = realloc(arena->memory, capacity);
    arena->cap = capacity;
    arena->memory = new;
}

void arena_reset(struct Arena *arena) {
    assert(arena);
    arena->at = 0;
}
