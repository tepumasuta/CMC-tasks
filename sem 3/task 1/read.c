#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define ALLOC_SIZE 2048

/* For showcasing purposes*/
void *dummy_malloc(size_t __size) {
    (void)__size;
    return NULL;
}
void *dummy_realloc(void *__ptr, size_t __size) {
    (void)__ptr;
    (void)__size;
    return NULL;
}
void *dummy_double_realloc(void *__ptr, size_t __size) {
    static bool used = false;
    if (used) return NULL;
    used = true;
    return realloc(__ptr, __size);
}
// #define malloc dummy_malloc
// #define realloc dummy_realloc
// #define realloc dummy_double_realloc

void *try_alloc(size_t size, size_t *allocated) {
    void *mem = NULL;
    while ((mem = malloc(size)) == NULL && (size /= 2) >= 4);
    if (mem && allocated)
        *allocated = size;
    return mem;
}

void *try_realloc(void *old_mem, size_t size, size_t additional, size_t *reallocated) {
    void *mem = NULL;
    while ((mem = realloc(old_mem, size + additional)) == NULL && (additional /= 2) >= 4);
    if (mem && reallocated)
        *reallocated = size + additional;
    return mem;
}


char *read_string(bool *failed) {
    size_t cap, size = 0;
    char *buf = try_alloc(ALLOC_SIZE, &cap);
    if (!buf) {
        if (failed)
            *failed = true;
        return NULL;
    }

    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        if (size == cap - 1) {
            char *new_buf = try_realloc(buf, cap, ALLOC_SIZE, &cap);
            if (!new_buf) {
                if (failed)
                    *failed = true;
                buf[size] = '\0';
                return buf;
            }
	    buf = new_buf;
        }
        buf[size++] = c;
    }

    buf[size] = '\0';
    if (failed)
        *failed = false;
    return buf;
}

int main(void) {
    bool failed;
    char *string = read_string(&failed);
    if (failed)
        puts(failed ? "Failed\n" : "Succsess\n");

    if (string) {
        printf("Read `%s`\n", string);
        free(string);
    }
    
    return 0;
}
