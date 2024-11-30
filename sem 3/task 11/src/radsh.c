/* TODO: Argument * expansion */
/* TODO: fg, bg */

#include <stdlib.h>

#include "arena.h"
#include "defer.h"
#include "lexer.h"
#include "parser.h"
#include "shell.h"
#include "repl.h"

#include <stdio.h>
#include <assert.h>
#include <errno.h>

#define ERROR_PRINT_SKIP(MSG) do {\
    repl_print_error(settings, MSG);\
    shell->last_exit_code = 1;\
    goto next_iteration;\
} while (0)

void arena_static_destroy_wrapper(void *arg) {
    if (arg == NULL) return;
    struct ArenaStatic *arena = *(struct ArenaStatic **)arg;
    if (arena) arena_static_try_destroy(&arena);
}

void arena_dynamic_destroy_wrapper(void *arg) {
    if (arg == NULL) return;
    struct ArenaDynamic *arena = arg;
    arena_dynamic_destroy(&arena);
}

void ast_node_shell_free_wrapper(void *arg) {
    if (arg == NULL) return;
    struct NodeShell *node = *(struct NodeShell **)arg;
    ast_node_shell_free(node);
}

void shell_destroy_wrapper(void *arg) {
    struct Shell *shell = arg;
    if (shell) shell_destroy(shell);
}

void arena_static_try_destroy_wrapper(void *arg) {
    struct ArenaStatic *arena = *(struct ArenaStatic **)arg;
    if (arena) arena_static_try_destroy(&arena);
}

struct DeferSystem defer_system;

void cleanup(void) {
    defer_system_do_defer(&defer_system);
    defer_system_deinit(&defer_system);
}

static struct ArenaStatic *symtable = NULL;
static struct AST ast = { NULL, NULL };

int main(void) {
    if (!defer_system_try_init(&defer_system, NULL)) abort();
    if (atexit(cleanup)) abort();

    struct ArenaDynamic *arena = arena_dynamic_create(64);
    defer_system_register(&defer_system, (lambda_t){ .func=arena_dynamic_destroy_wrapper, .data=arena });

    defer_system_register(&defer_system, (lambda_t){ .func=arena_static_destroy_wrapper, .data=&symtable });
    token_view_t lexed_tokens;
    struct ArenaDynamic *token_allocator = arena_dynamic_try_create(10);
    defer_system_register(&defer_system, (lambda_t){ .func=arena_dynamic_destroy_wrapper, .data=token_allocator });

    defer_system_register(&defer_system, (lambda_t){ .func=ast_node_shell_free_wrapper, .data=&ast.root });
    defer_system_register(&defer_system, (lambda_t){ .func=arena_static_try_destroy_wrapper, .data=&ast.string_allocator });

    struct REPLSettings settings = { .colorized=true };
    enum ShellError shell_error = SHELL_ERROR_NONE;
    struct Shell *shell = shell_create(&shell_error);
    if (shell_error != SHELL_ERROR_NONE) assert(0 && "TODO: Shell error");
    defer_system_register(&defer_system, (lambda_t){ .func=shell_destroy_wrapper, .data=shell });

    char *string;
    enum ReplError error = REPL_ERROR_NONE;
    enum LexerError lex_error = LEXER_ERROR_NONE;
    enum ParserError parse_error = PARSER_ERROR_NONE;
    repl_colorful_welcome();
    repl_print_input_promt(shell, settings);
    errno = 0;
    while ((string = repl_read_string(arena, &error)) || errno == EINTR) {
        if (errno == EINTR) {
            if (sigint_signal) { printf("^C"); sigint_signal = false; }
            if (sigtstp_signal) { printf("^Z"); sigtstp_signal = false; }
            goto next_iteration;
        }
        arena_dynamic_reset(token_allocator);
        string_view_t lexer_view = { .start=string, .length=arena->at };
        if (sv_ends_with(lexer_view, SV_FROM_CSTR("\n"))) sv_chop_end(&lexer_view);
        lex_error = lexer_lex(lexer_view, &symtable, &lexed_tokens, token_allocator);
        switch (lex_error) {
        case LEXER_ERROR_NO_MATCHING_QUOT:
            // TODO: lexer try to reread the string
            ERROR_PRINT_SKIP("Lexer error, unclosed quote");
        case LEXER_ERROR_FAILED_TO_ALLOC:
            repl_print_fatal(settings, "Lexer failed to alloc, unable to operate");
            exit(1);
        case LEXER_ERROR_NONE: break;
        }

        parse_error = parser_parse(lexed_tokens, &ast, symtable);
        switch (parse_error) {
        case PARSER_ERROR_NONE: break;
        case PARSER_ERROR_EMPTY: goto next_iteration;
        case PARSER_ERROR_FAILED_TO_ALLOC:
            repl_print_fatal(settings, "Parser failed to alloc, unable to operate");
            exit(1);
        case PARSER_ERROR_UNCLOSED_PAREN:
            // TODO: parser try to reread the string
            ERROR_PRINT_SKIP("Parser error, unclosed parenthesis");
        case PARSER_ERROR_DOUBLE_REDIRECTION: ERROR_PRINT_SKIP("Parser error, double redirection detected, ambigious command");
        case PARSER_ERROR_INCOMPLETE_REDIRECTION: ERROR_PRINT_SKIP("Parser error, incomplete redirection detected, ambigious command");
        case PARSER_ERROR_NONSHELL_INSIDE_PAREN: ERROR_PRINT_SKIP("Parser error, failed to parse command inside parenthesis");
        case PARSER_ERROR_SHELL_LEFTOVER: ERROR_PRINT_SKIP("Parser error, failed to parse command fully, some leftover");
        case PARSER_ERROR_INVALID_PIPE: ERROR_PRINT_SKIP("Parser error, failed to parse pipe");
        case PARSER_ERROR_INVALID_PAREN_CLOSE_FIRST: ERROR_PRINT_SKIP("Parser error, closing parenthesis before opening one");
        case PARSER_ERROR_INVALID_CONDITIONAL: ERROR_PRINT_SKIP("Parser error, no command after || or &&");
        case PARSER_ERROR_INVALID_REDIRECTION: ERROR_PRINT_SKIP("Parser error, invalid redirection expression");
        }
    
        shell_execute(shell, &ast, &shell_error);
        switch (shell_error) {
        case SHELL_ERROR_NONE: break;
        case SHELL_ERROR_FAILED_TO_ALLOC:
            repl_print_fatal(settings, "Shell failed to alloc, unable to operate");
            exit(1);
        case SHELL_ERROR_GENERAL_SYSCALL: ERROR_PRINT_SKIP("Shell error, some syscall failed");
        case SHELL_ERROR_TOO_MANY_BG: ERROR_PRINT_SKIP("Shell error, too many background processes");
        case SHELL_ERROR_FAILED_TO_CLOSE: ERROR_PRINT_SKIP("Shell error, failed to close file descriptor");
        case SHELL_ERROR_FAILED_TO_FORK: ERROR_PRINT_SKIP("Shell error, failed to give birth (fork)");
        case SHELL_ERROR_LOGIN_CAP: assert(0 && "Unreachable");
        }
next_iteration:
        arena_static_try_destroy(&symtable);
        arena_static_try_destroy(&ast.string_allocator);
        ast_node_shell_free(ast.root);
        ast.root = NULL;
        repl_print_input_promt(shell, settings);
    }

    return 0;
}
