#ifndef REPL_H
#define REPL_H 1

#include "arena.h"
#include "shell.h"

enum ReplError {
    REPL_ERROR_NONE = 0,
    REPL_ERROR_FAILED_TO_ALLOC,
};

struct REPLSettings {
    bool colorized;
};

char *repl_read_string(struct ArenaDynamic *allocator, enum ReplError *error);
void repl_print_input_promt(struct Shell *shell, const struct REPLSettings settings);
void repl_print_fatal(struct REPLSettings settings, const char *message);
void repl_print_error(struct REPLSettings settings, const char *message);
#if 0
void repl_print_info(const struct REPLSettings settings);
void repl_print_warning(const struct REPLSettings settings);
#endif

#endif
