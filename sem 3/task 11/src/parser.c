#include "parser.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define error_return(ERRVALUE) do { *error = (ERRVALUE); goto on_error; } while(0)

static struct NodeShell *try_parse_shell_command(
    token_view_t tokens, struct ArenaStatic *string_allocator,
    enum ParserError *error, size_t *chop_count, bool new_shell
);

static struct NodeBasicCommand *try_parse_basic_command(
    token_view_t tokens, struct ArenaStatic *string_allocator,
    size_t *chop_count, enum ParserError *error
) {
    assert(string_allocator);
    assert(chop_count);
    if (!tokens.length || tokens.start->type != TOKEN_TYPE_SYMBOL) {
        *error = PARSER_ERROR_EXPECTED_SYMBOL;
        return NULL;
    }
    size_t i, total_args_length = 0;

    for (i = 1; i < tokens.length; i++) {
        if (tokens.start[i].type != TOKEN_TYPE_SYMBOL)
            break;
        total_args_length += tokens.start[i].as_symbol.symbol.length + 1;
    }
    *chop_count = i;

    char **args = arena_static_alloc(string_allocator, sizeof(*args) * i);
    args[i - 1] = NULL;
    for (size_t j = 0; j < i - 1; j++) {
        args[j] = arena_static_alloc(string_allocator, tokens.start[j + 1].as_symbol.symbol.length + 1);
        memcpy(args[j],
               tokens.start[j + 1].as_symbol.symbol.start,
               tokens.start[j + 1].as_symbol.symbol.length + 1);
    }
    char *command = arena_static_alloc(string_allocator, tokens.start->as_symbol.symbol.length + 1);
    memcpy(command,
            tokens.start->as_symbol.symbol.start,
            tokens.start->as_symbol.symbol.length + 1);

    struct NodeBasicCommand *node = malloc(sizeof(*node));
    if (!node) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
    *node = (struct NodeBasicCommand){
        .name = command,
        .args = args,
    };
    return node;
on_error:
    return NULL;
}

static struct NodePipeline *try_parse_pipeline(
    token_view_t tokens, struct ArenaStatic *string_allocator,
    enum ParserError *error, size_t *chop_count
) {
    assert(string_allocator);
    assert(error);
    assert(chop_count);
    size_t count;
    struct NodeBasicCommand *command = NULL;
    struct NodePipeline *node = NULL;
    command = try_parse_basic_command(tokens, string_allocator, &count, error);
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (!command) return NULL;
    tv_chop_n(&tokens, count);
    node = malloc(sizeof(*node));
    if (!node) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
    if (!tokens.length || tokens.start->type != TOKEN_TYPE_OPERATOR
        || tokens.start->as_operator != TOKEN_OPERATOR_PIPE
    ) {
        *node = (struct NodePipeline){ .command = command, .pipe = NULL };
        *chop_count = count;
        return node;
    } else {
        tv_chop(&tokens);
        size_t additional_count;
        struct NodePipeline *pipe_node = try_parse_pipeline(tokens, string_allocator, error, &additional_count);
        if (*error != PARSER_ERROR_NONE) error_return(*error);
        if (!pipe_node) error_return(PARSER_ERROR_INVALID_PIPE);
        *node = (struct NodePipeline) { .command = command, .pipe = pipe_node };
        *chop_count = count + additional_count + 1;
        return node;
    }
    assert(0 && "Unreachable");
on_error:
    if (command) ast_node_basic_command_free(command);
    if (node) free(node);
    return NULL;
}

static bool try_parse_redirection_input(token_view_t tokens, char **input, enum ParserError *error) {
    assert(input);
    assert(error);
    if (!tokens.length) return false;
    if (tokens.start->type != TOKEN_TYPE_OPERATOR) return false;
    if (tokens.start->as_operator != TOKEN_OPERATOR_ALBR) return false;
    if (tokens.length == 1) {
        *error = PARSER_ERROR_INCOMPLETE_REDIRECTION;
        return true;
    }
    if (tokens.start[1].type != TOKEN_TYPE_SYMBOL) {
        *error = PARSER_ERROR_INVALID_REDIRECTION;
        return true;
    }
    *input = (char *)tokens.start[1].as_symbol.symbol.start;
    return true;
}

static bool try_parse_redirection_output(token_view_t tokens, char **output, enum ParserError *error) {
    assert(output);
    assert(error);
    if (!tokens.length) return false;
    if (tokens.start->type != TOKEN_TYPE_OPERATOR) return false;
    if (tokens.start->as_operator != TOKEN_OPERATOR_ARBR) return false;
    if (tokens.length == 1) {
        *error = PARSER_ERROR_INCOMPLETE_REDIRECTION;
        return true;
    }
    if (tokens.start[1].type != TOKEN_TYPE_SYMBOL) {
        *error = PARSER_ERROR_INVALID_REDIRECTION;
        return true;
    }
    *output = (char *)tokens.start[1].as_symbol.symbol.start;
    return true;
}

static bool try_parse_redirection_append(token_view_t tokens, char **output, enum ParserError *error) {
    assert(output);
    assert(error);
    if (!tokens.length) return false;
    if (tokens.start->type != TOKEN_TYPE_OPERATOR) return false;
    if (tokens.start->as_operator != TOKEN_OPERATOR_DARBR) return false;
    if (tokens.length == 1) {
        *error = PARSER_ERROR_INCOMPLETE_REDIRECTION;
        return true;
    }
    if (tokens.start[1].type != TOKEN_TYPE_SYMBOL) {
        *error = PARSER_ERROR_INVALID_REDIRECTION;
        return true;
    }
    *output = (char *)tokens.start[1].as_symbol.symbol.start;
    return true;
}

static bool try_parse_redirection(
    token_view_t tokens, char **input, char **output, bool *append,
    enum ParserError *error, size_t *chop_count
) {
    assert(input);
    assert(output);
    assert(append);
    assert(error);
    bool was_input = false, was_output = false, was_append = false;
    size_t count = 0;
    for (;;) {
        if (try_parse_redirection_input(tokens, input, error)) {
            if (*error != PARSER_ERROR_NONE) return true;
            if (was_input) {
                *error = PARSER_ERROR_DOUBLE_REDIRECTION;
                return true;
            }
            count += 2;
            was_input = true;
        } else if(try_parse_redirection_output(tokens, output, error)) {
            if (*error != PARSER_ERROR_NONE) return true;
            if (was_output || was_append) {
                *error = PARSER_ERROR_DOUBLE_REDIRECTION;
                return true;
            }
            count += 2;
            was_output = true;
        } else if(try_parse_redirection_append(tokens, output, error)) {
            if (*error != PARSER_ERROR_NONE) return true;
            if (was_output || was_append) {
                *error = PARSER_ERROR_DOUBLE_REDIRECTION;
                return true;
            }
            count += 2;
            was_append = true;
        } else if (count == 0) return false;
        else break;
        tv_chop_n(&tokens, 2);
    }
    *append = was_append;
    if (!was_input) *input = NULL;
    if (!was_output && ! was_append) *output = NULL;
    *chop_count = count;
    assert(count <= 4);
    return true;
}

static bool try_parse_redirection_end(token_view_t tokens, char **output, enum ParserError *error, bool *append) {
    char *new_output;
    if (try_parse_redirection_output(tokens, &new_output, error)) {
        if (*error != PARSER_ERROR_NONE) return true;
        tv_chop_n(&tokens, 2);
        if (try_parse_redirection_append(tokens, &new_output, error)) {
            if (*error != PARSER_ERROR_NONE) return true;
            *error = PARSER_ERROR_DOUBLE_REDIRECTION;
            return true;
        }
        *append = false;
        *output = new_output;
        return true;
    }
    if (try_parse_redirection_append(tokens, &new_output, error)) {
        if (*error != PARSER_ERROR_NONE) return true;
        tv_chop_n(&tokens, 2);
        if (try_parse_redirection_output(tokens, &new_output, error)) {
            if (*error != PARSER_ERROR_NONE) return true;
            *error = PARSER_ERROR_DOUBLE_REDIRECTION;
            return true;
        }
        *append = true;
        *output = new_output;
        return true;
    }
    return false;
}

static struct NodeCommand *try_parse_command(
    token_view_t tokens, struct ArenaStatic *string_allocator,
    enum ParserError *error, size_t *chop_count
) {
    assert(string_allocator);
    assert(error);
    assert(chop_count);
    char *input = NULL, *output = NULL;
    bool append;
    size_t count, additional_count;
    struct NodePipeline *pipe = NULL;
    struct NodeCommand *node = NULL;
    struct NodeShell *shell = NULL;
    if (try_parse_redirection(tokens, &input, &output, &append, error, &count)) {
        if (*error != PARSER_ERROR_NONE) error_return(*error);
        tv_chop_n(&tokens, count);
        enum NodeCommandType type;
        if (input != NULL && output != NULL) { type = NODE_COMMAND_IO_PIPE; }
        else if (input == NULL && output != NULL) { error_return(PARSER_ERROR_INVALID_COMMAND_START); }
        else { type = NODE_COMMAND_I_PIPE_O; }
        pipe = try_parse_pipeline(tokens, string_allocator, error, &additional_count);
        if (*error != PARSER_ERROR_NONE) error_return(*error);
        if (!pipe) error_return(PARSER_ERROR_INVALID_COMMAND);
        tv_chop_n(&tokens, additional_count);
        node = malloc(sizeof(*node));
        if (!node) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
        struct NodeIOPipe *ionode = malloc(sizeof(*ionode));
        if (!ionode) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
        *ionode = (struct NodeIOPipe){ .append = append, .input = input, .output = output, .pipe = pipe };
        *node = (struct NodeCommand){ .type = type, .io_pipe = ionode };
        if (type == NODE_COMMAND_IO_PIPE) {
            *chop_count = count + additional_count;
            return node;
        }
        if (try_parse_redirection_end(tokens, &output, error, &append)) {
            if (*error != PARSER_ERROR_NONE) error_return(*error);
            node->io_pipe->output = output;
            node->io_pipe->append = append;
            additional_count += 2;
        }
        *chop_count = count + additional_count;
        return node;
    }
    if (pipe = try_parse_pipeline(tokens, string_allocator, error, &count)) {
        tv_chop_n(&tokens, count);
        node = malloc(sizeof(*node));
        if (!node) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
        struct NodeIOPipe *ionode = malloc(sizeof(*ionode));
        if (!ionode) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
        if (try_parse_redirection(tokens, &input, &output, &append, error, &additional_count)) {
            if (*error != PARSER_ERROR_NONE) error_return(*error);
        } else {
            additional_count = 0;
        }
        *ionode = (struct NodeIOPipe){ .append = append, .input = input, .output = output, .pipe = pipe };
        *node = (struct NodeCommand){ .type = NODE_COMMAND_PIPE_IO, .io_pipe = ionode };
        *chop_count = count + additional_count;
        return node;
    }
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (tokens.length > 0 && tokens.start->type == TOKEN_TYPE_SYNTAX) {
        if (tokens.start->as_syntax == TOKEN_SYNTAX_CPAREN)
            error_return(PARSER_ERROR_INVALID_PAREN_CLOSE_FIRST);
        if (tokens.start->as_syntax != TOKEN_SYNTAX_OPAREN) return NULL;
        size_t at;
        int parenthesis = 1;
        for (at = 1; at < tokens.length; at++) {
            if (tokens.start[at].type == TOKEN_TYPE_SYNTAX) {
                switch (tokens.start[at].as_syntax) {
                case TOKEN_SYNTAX_OPAREN: parenthesis++; break;
                case TOKEN_SYNTAX_CPAREN: parenthesis--; break;
                case TOKEN_SYNTAX_SIZE: assert(0 && "Unreachable");
                default:
                }
                if (!parenthesis) break;
            }
        }
        if (at == tokens.length) error_return(PARSER_ERROR_UNCLOSED_PAREN);
        shell = try_parse_shell_command(
            (token_view_t){ .start=tokens.start + 1, .length = at - 1 },
            string_allocator, error, &count, true
        );
        if (*error != PARSER_ERROR_NONE) error_return(*error);
        if (!shell) error_return(PARSER_ERROR_NONSHELL_INSIDE_PAREN);
        node = malloc(sizeof(*node));
        if (!node) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
        *node = (struct NodeCommand){ .type = NODE_COMMAND_SHELL, .command=shell };
        *chop_count = count + 2;
        return node;
    }
    return NULL;
on_error:
    if (pipe) ast_node_pipeline_free(pipe);
    if (node) free(node);
    if (shell) ast_node_shell_free(shell);
    return NULL;
}

static struct NodeConditional *try_parse_conditional(
    token_view_t tokens, struct ArenaStatic *string_allocator,
    enum ParserError *error, size_t *chop_count
) {
    assert(string_allocator);
    assert(error);
    assert(chop_count);
    size_t count;
    struct NodeCommand *command = NULL;
    struct NodeConditional *node = NULL;
    command = try_parse_command(tokens, string_allocator, error, &count);
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (!command) return NULL;
    tv_chop_n(&tokens, count);
    node = malloc(sizeof(*node));
    if (!node) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
    if (!tokens.length || tokens.start->type != TOKEN_TYPE_OPERATOR) {
        *node = (struct NodeConditional){ .command=command, .next=NULL };
        *chop_count = count;
        return node;
    }
    switch (tokens.start->as_operator) {
    case TOKEN_OPERATOR_AND: node->if_false = false; break;
    case TOKEN_OPERATOR_OR: node->if_false = true; break;
    case TOKEN_OPERATOR_SIZE: assert(0 && "Unreachable");
    default:
        *node = (struct NodeConditional){ .command=command, .next=NULL };
        *chop_count = count;
        return node;
    }
    tv_chop(&tokens);
    size_t additional_count;
    struct NodeConditional *next = try_parse_conditional(tokens, string_allocator, error, &additional_count);
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (!next) error_return(PARSER_ERROR_INVALID_CONDITIONAL);
    node->command = command;
    node->next = next;
    *chop_count = count + additional_count + 1;
    return node;
on_error:
    if (command) ast_node_command_free(command);
    if (node) free(node);
    return NULL;
}

static struct NodeShell *try_parse_shell_command(
    token_view_t tokens, struct ArenaStatic *string_allocator,
    enum ParserError *error, size_t *chop_count, bool new_shell
) {
    assert(string_allocator);
    assert(error);
    assert(chop_count);
    size_t count;
    struct NodeConditional *conditional = NULL;
    struct NodeShell *node = NULL;
    struct NodeShell *shell = NULL;
    conditional = try_parse_conditional(tokens, string_allocator, error, &count);
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (!conditional) return NULL;
    tv_chop_n(&tokens, count);
    node = malloc(sizeof(*node));
    if (!node) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
    if (!tokens.length || tokens.start->type != TOKEN_TYPE_OPERATOR) {
        *node = (struct NodeShell){ .bg=false, .command=conditional, .new_shell=new_shell, .next=NULL };
        *chop_count = count;
        return node;
    }
    bool background;
    switch (tokens.start->as_operator) {
    case TOKEN_OPERATOR_AMP: background = true; break;
    case TOKEN_OPERATOR_SEMI: background = false; break;
    case TOKEN_OPERATOR_SIZE: assert(0 && "Unreachable");
    default: error_return(PARSER_ERROR_SHELL_LEFTOVER);
    }
    tv_chop(&tokens);
    if (!tokens.length) {
        *node = (struct NodeShell){ .bg=background, .command=conditional, .new_shell=new_shell, .next=NULL };
        *chop_count = count + 1;
        return node;
    }
    size_t additional_count;
    shell = try_parse_shell_command(tokens, string_allocator, error, &additional_count, false);
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (!shell) {
        *node = (struct NodeShell){ .bg=background, .command=conditional, .new_shell=new_shell, .next=NULL };
        *chop_count = count + 1;
        return node;
    }
    tv_chop_n(&tokens, additional_count);
    if (!tokens.length) {
        *node = (struct NodeShell){ .bg=background, .command=conditional, .new_shell=new_shell, .next=shell };
        *chop_count = count + additional_count + 1;
        return node;
    }
    error_return(PARSER_ERROR_SHELL_LEFTOVER);
on_error:
    if (conditional) ast_node_conditional_free(conditional);
    if (node) free(node);
    if (shell) ast_node_shell_free(shell);
    return NULL;
}

enum ParserError parser_parse(token_view_t tokens, struct AST *parsed_ast,
                              const struct ArenaStatic *symtable) {
    if (!tokens.length) return PARSER_ERROR_EMPTY;
    size_t above_buf = 0;
    for (size_t i = 0; i < tokens.length; i++)
        if (tokens.start[i].type == TOKEN_TYPE_SYMBOL)
            above_buf += sizeof(char *);
    above_buf *= 2;
    struct ArenaStatic *allocator = arena_static_create(above_buf + symtable->at);
    size_t count;
    enum ParserError error = PARSER_ERROR_NONE;
    struct NodeShell *node = try_parse_shell_command(tokens, allocator, &error, &count, false);
    if (error != PARSER_ERROR_NONE) {
        arena_static_destroy(&allocator);
        ast_node_shell_free(node);
        return error;
    }
    if (!node) assert(0 && "Unexpected");
    *parsed_ast = (struct AST){
        .root = node,
        .string_allocator = allocator,
    };
    return PARSER_ERROR_NONE;
}
