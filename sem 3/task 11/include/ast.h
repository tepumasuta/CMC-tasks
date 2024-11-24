#ifndef AST_H
#define AST_H 1

#include "arena.h"


struct NodeShell {
    bool new_shell;
    bool bg;
    struct NodeConditional *command;
    struct NodeShell *next;
};

struct NodeConditional {
    struct NodeCommand *command;
    bool if_false;
    struct NodeConditional *next;
};

enum NodeCommandType {
    NODE_COMMAND_IO_PIPE = 0,
    NODE_COMMAND_I_PIPE_O,
    NODE_COMMAND_PIPE_IO,
    NODE_COMMAND_SHELL,
    NODE_COMMAND_TYPE_SIZE,
};

struct NodeIOPipe {
    char *input, *output;
    bool append;
    struct NodePipeline *pipe;
};

struct NodeCommand {
    enum NodeCommandType type;
    union {
        struct NodeIOPipe *io_pipe;
        struct NodeShell *command;
    };
};

struct NodeBasicCommand {
    char *name;
    char **args;
};

struct NodePipeline {
    struct NodeBasicCommand *command;
    struct NodePipeline *pipe;
};

struct AST {
    struct NodeShell *root;
    struct ArenaStatic *string_allocator;
};

void ast_node_shell_free(struct NodeShell *node);
void ast_node_pipeline_free(struct NodePipeline *node);
void ast_node_basic_command_free(struct NodeBasicCommand *node);
void ast_node_command_free(struct NodeCommand *node);
void ast_node_io_pipe_free(struct NodeIOPipe *node);
void ast_node_conditional_free(struct NodeConditional *node);

void ast_print_node_shell(struct NodeShell *node);
void ast_print_node_pipeline(struct NodePipeline *node);
void ast_print_node_basic_command(struct NodeBasicCommand *node);
void ast_print_node_command(struct NodeCommand *node);
void ast_print_node_io_pipe(struct NodeIOPipe *node);
void ast_print_node_conditional(struct NodeConditional *node);

#endif
