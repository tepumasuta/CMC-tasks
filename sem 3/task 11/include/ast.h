#ifndef AST_H
#define AST_H 1

#include "arena.h"

struct AST {
    struct NodeShell *root;
    struct ArenaStatic *string_allocator;
};

struct NodeShell {
    bool new_shell;
    bool bg;
    char *input, *output;
    bool append;
    struct NodeConditional *command;
    struct NodeShell *next;
};

struct NodeConditional {
    bool many;
    union {
        struct NodePipeline *commands;
        struct NodeBasicCommand *command;
    };
    struct NodeConditional *next;
    bool if_false;
};

enum NodeBasicity {
    NODE_BASICITY_BASIC = 0,
    NODE_BASICITY_SHELL,
};

struct NodePipeline {
    enum NodeBasicity type;
    union {
        struct NodeBasicCommand *command;
        struct NodeShell *shell;
    };
    struct NodePipeline *pipe;
};

struct NodeBasicCommand {
    char *name;
    char **args;
    char *input, *output;
    bool append;
};

void ast_node_shell_free(struct NodeShell *node);
void ast_node_pipeline_free(struct NodePipeline *node);
void ast_node_basic_command_free(struct NodeBasicCommand *node);
void ast_node_conditional_free(struct NodeConditional *node);

void ast_print_node_shell(struct NodeShell *node);
void ast_print_node_pipeline(struct NodePipeline *node);
void ast_print_node_basic_command(struct NodeBasicCommand *node);
void ast_print_node_conditional(struct NodeConditional *node);

#endif
