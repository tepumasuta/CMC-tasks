#include "parser.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define error_return(ERRVALUE) do { *error = (ERRVALUE); goto on_error; } while(0)


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
    if (*input != NULL) {
        *error = PARSER_ERROR_DOUBLE_REDIRECTION;
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
    if (*output != NULL) {
        *error = PARSER_ERROR_DOUBLE_REDIRECTION;
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
    if (*output != NULL) {
        *error = PARSER_ERROR_DOUBLE_REDIRECTION;
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

static struct NodeBasicCommand *try_parse_basic_command(
    token_view_t tokens, struct ArenaStatic *string_allocator,
    size_t *chop_count, enum ParserError *error
) {
    assert(string_allocator);
    assert(chop_count);
    if (!tokens.length
        || tokens.start->type != TOKEN_TYPE_SYMBOL
        && tokens.start->type != TOKEN_TYPE_OPERATOR
        && !token_is_redirection(tokens.start->as_operator))
    {
        return NULL;
    }

    size_t i = 0, total_args_length = 0, symbols = 0;
    char *input = NULL, *output = NULL;
    bool append = false;
    while (
        i < tokens.length && (tokens.start[i].type == TOKEN_TYPE_SYMBOL
        || tokens.start[i].type == TOKEN_TYPE_OPERATOR
        && token_is_redirection(tokens.start[i].as_operator))
    ) {
        if (tokens.start[i].type == TOKEN_TYPE_SYMBOL) {
            total_args_length += tokens.start[i].as_symbol.symbol.length + 1;
            i++;
            symbols++;
        } else {
            token_view_t now = (token_view_t){ .start = tokens.start + i, .length = tokens.length - i };
            size_t count = 0;
            bool result = try_parse_redirection(now, &input, &output, &append, error, &count);
            if (*error != PARSER_ERROR_NONE) error_return(*error);
            i += count;
        }
    }
    *chop_count = i;

    char **args = arena_static_alloc(string_allocator, sizeof(*args) * (symbols + 2));
    args[symbols] = NULL;
    size_t k = 0;
    for (size_t j = 0; j < i;) {
        if (tokens.start[j].type == TOKEN_TYPE_OPERATOR) { j += 2; continue; }
        assert(k < symbols);
        args[k] = arena_static_alloc(string_allocator, tokens.start[j].as_symbol.symbol.length + 1);
        memcpy(args[k],
               tokens.start[j].as_symbol.symbol.start,
               tokens.start[j].as_symbol.symbol.length + 1);
        k++;
        j++;
    }
    char *command = args[0];

    struct NodeBasicCommand *node = malloc(sizeof(*node));
    if (!node) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
    *node = (struct NodeBasicCommand){
        .name = command,
        .args = args,
        .input = input,
        .output = output,
        .append = append,
    };
    return node;
on_error:
    return NULL;
}

static struct NodeShell *try_parse_shell_command(
    token_view_t tokens, struct ArenaStatic *string_allocator,
    enum ParserError *error, size_t *chop_count, bool new_shell
);

static struct NodeShell *try_parse_redirected_shell_command(
    token_view_t tokens, struct ArenaStatic *string_allocator,
    enum ParserError *error, size_t *chop_count
) {
    assert(string_allocator);
    assert(error);
    char *input = NULL, *output = NULL;
    bool append = false;
    struct NodeShell *node = NULL;
    size_t count = 0;
    size_t additional_count = 0;
    if (try_parse_redirection(tokens, &input, &output, &append, error, &additional_count))
        if (*error != PARSER_ERROR_NONE) error_return(*error);
    tv_chop_n(&tokens, additional_count);
    if (tokens.start->as_syntax == TOKEN_SYNTAX_CPAREN) error_return(PARSER_ERROR_INVALID_PAREN_CLOSE_FIRST);
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
    count += additional_count;
    node = try_parse_shell_command(
        (token_view_t){ .start=tokens.start + 1, .length = at - 1 },
        string_allocator, error, &additional_count, true
    );
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (!node) error_return(PARSER_ERROR_NONSHELL_INSIDE_PAREN);
    tv_chop_n(&tokens, additional_count + 2);
    count += additional_count + 2;
    additional_count = 0;
    if (try_parse_redirection(tokens, &input, &output, &append, error, &additional_count))
        if (*error != PARSER_ERROR_NONE) error_return(*error);
    node->append = append;
    node->input = input;
    node->output = output;
    *chop_count = count + additional_count;
    return node;
on_error:
    if (node) ast_node_shell_free(node);
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
    struct NodeShell *shell = NULL;
    struct NodePipeline *node = NULL;
    command = try_parse_basic_command(tokens, string_allocator, &count, error);
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (!command) {
        shell = try_parse_redirected_shell_command(tokens, string_allocator, error, &count);
        if (*error != PARSER_ERROR_NONE) error_return(*error);
    }
    tv_chop_n(&tokens, count);
    node = malloc(sizeof(*node));
    if (!node) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
    if (!tokens.length || tokens.start->type != TOKEN_TYPE_OPERATOR
        || tokens.start->as_operator != TOKEN_OPERATOR_PIPE
    ) {
        node->pipe = NULL;
        node->type = command ? NODE_BASICITY_BASIC : NODE_BASICITY_SHELL;
        if (command) node->command = command;
        else node->shell = shell;
        *chop_count = count;
        return node;
    }
    tv_chop(&tokens);
    size_t additional_count;
    struct NodePipeline *pipe_node = try_parse_pipeline(tokens, string_allocator, error, &additional_count);
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (!pipe_node) error_return(PARSER_ERROR_INVALID_PIPE);
    node->pipe = pipe_node;
    node->type = command ? NODE_BASICITY_BASIC : NODE_BASICITY_SHELL;
    if (command) node->command = command;
    else node->shell = shell;
    *chop_count = count + additional_count + 1;
    return node;
on_error:
    if (shell) ast_node_shell_free(shell);
    if (command) ast_node_basic_command_free(command);
    if (node) free(node);
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
    struct NodePipeline *pipeline = NULL;
    struct NodeConditional *node = NULL;
    pipeline = try_parse_pipeline(tokens, string_allocator, error, &count);
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (!pipeline) return NULL;
    tv_chop_n(&tokens, count);
    node = malloc(sizeof(*node));
    if (!node) error_return(PARSER_ERROR_FAILED_TO_ALLOC);
    if (pipeline->type == NODE_BASICITY_BASIC && !pipeline->pipe) {
        node->many = false;
        node->command = pipeline->command;
        pipeline->command = NULL;
        ast_node_pipeline_free(pipeline);
    } else {
        node->many = true;
        node->commands = pipeline;
    }
    if (!tokens.length || tokens.start->type != TOKEN_TYPE_OPERATOR) {
        node->next = NULL;
        *chop_count = count;
        return node;
    }
    switch (tokens.start->as_operator) {
    case TOKEN_OPERATOR_AND: node->if_false = false; break;
    case TOKEN_OPERATOR_OR: node->if_false = true; break;
    case TOKEN_OPERATOR_SIZE: assert(0 && "Unreachable");
    default:
        node->next = NULL;
        *chop_count = count;
        return node;
    }
    tv_chop(&tokens);
    size_t additional_count;
    struct NodeConditional *next = try_parse_conditional(tokens, string_allocator, error, &additional_count);
    if (*error != PARSER_ERROR_NONE) error_return(*error);
    if (!next) error_return(PARSER_ERROR_INVALID_CONDITIONAL);
    node->next = next;
    *chop_count = count + additional_count + 1;
    return node;
on_error:
    if (pipeline) ast_node_pipeline_free(pipeline);
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
    above_buf *= 3;
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
