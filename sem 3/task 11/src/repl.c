#include "repl.h"
#include "shell.h"

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

char *repl_read_string(struct ArenaDynamic *allocator, enum ReplError *error) {
    assert(allocator);
    assert(error);
    assert(allocator->cap >= 2);
    arena_dynamic_reset(allocator);
    char *last_write;
    errno = 0;
    while (last_write = fgets(allocator->memory + allocator->at, allocator->cap, stdin)) {
        size_t more_length = strlen(last_write);
        allocator->at += more_length;
        if (((char *)allocator->memory)[allocator->at - 1] == '\n') break;
        if (!arena_dynamic_try_reserve(allocator, allocator->cap * 2)) {
            *error = REPL_ERROR_FAILED_TO_ALLOC;
            return NULL;
        }
        errno = 0;
    }
    if (!last_write && errno != 0) { assert(0 && "TODO: Handle error"); }
    return allocator->at ? allocator->memory : NULL;
}

void repl_print_input_promt(struct Shell *shell, const struct REPLSettings settings) {
    if (shell->last_exit_code) {
        if (settings.colorized) printf("\033[91m");
        printf("[%d]", shell->last_exit_code);
        if (settings.colorized) printf("\033[0m");
    }
    if (settings.colorized) printf("\033[92m");
    printf("%s", shell->login);
    if (settings.colorized) printf("\033[0m");
    printf("@");
    if (settings.colorized) printf("\033[94m");
    printf("%s", shell->cwd);
    if (settings.colorized) printf("\033[0m");
    printf(">");
    fflush(stdout);
}

void repl_print_fatal(struct REPLSettings settings, const char *message) {
    if (settings.colorized) fprintf(stderr, "\033[91m");
    fprintf(stderr, "[FATAL]: %s\n", message);
    if (settings.colorized) fprintf(stderr, "\033[0m");
}

void repl_print_error(struct REPLSettings settings, const char *message) {
    if (settings.colorized) fprintf(stderr, "\033[91m");
    fprintf(stderr, "[ERROR]: %s\n", message);
    if (settings.colorized) fprintf(stderr, "\033[0m");
}

void repl_printf_error(struct REPLSettings settings, const char *message, ...) {
    if (settings.colorized) fprintf(stderr, "\033[91m");
    fputs("[ERROR]: ", stderr);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    if (settings.colorized) fprintf(stderr, "\033[0m");
}
