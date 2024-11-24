#include "ast.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

void ast_node_shell_free(struct NodeShell *node) {
    if (!node) return;
    if (node->command) ast_node_conditional_free(node->command);
    if (node->next) ast_node_shell_free(node->next);
    free(node);
}

void ast_node_pipeline_free(struct NodePipeline *node) {
    if (!node) return;
    if (node->command) ast_node_basic_command_free(node->command);
    if (node->pipe) ast_node_pipeline_free(node->pipe);
    free(node);
}

void ast_node_basic_command_free(struct NodeBasicCommand *node) {
    if (!node) return;
    free(node);
}

void ast_node_command_free(struct NodeCommand *node) {
    if (!node) return;
    assert(node->type != NODE_COMMAND_TYPE_SIZE);
    if (node->type == NODE_COMMAND_SHELL) { if (node->command) ast_node_shell_free(node->command); }
    else if (node->io_pipe) ast_node_io_pipe_free(node->io_pipe);
    free(node);
}

void ast_node_io_pipe_free(struct NodeIOPipe *node) {
    if (!node) return;
    if (node->pipe) ast_node_pipeline_free(node->pipe);
    free(node);
}

void ast_node_conditional_free(struct NodeConditional *node) {
    if (!node) return;
    if (node->next) ast_node_conditional_free(node->next);
    if (node->command) ast_node_command_free(node->command);
    free(node);
}

void ast_print_node_shell(struct NodeShell *node) {
    if (!node) { printf("nil"); return; }
    printf("NodeShell{new_shell=%d,bg=%d,command=", node->new_shell, node->bg);
    ast_print_node_conditional(node->command);
    printf(",next=");
    ast_print_node_shell(node->next);
    printf("}");
}

void ast_print_node_pipeline(struct NodePipeline *node) {
    if (!node) { printf("nil"); return; }
    printf("NodePipeline{command=");
    ast_print_node_basic_command(node->command);
    printf(",pipe=");
    ast_print_node_pipeline(node->pipe);
    printf("}");
}

void ast_print_node_basic_command(struct NodeBasicCommand *node) {
    if (!node) { printf("nil"); return; }
    printf("NodeBasicCommand{name=`%s`,args=[", node->name);
    for (char **p = node->args; *p; p++)
        printf("`%s`,", *p);
    printf("]}");
}

void ast_print_node_command(struct NodeCommand *node) {
    if (!node) { printf("nil"); return; }
    printf("NodeCommand{type=");
    switch (node->type) {
    case NODE_COMMAND_IO_PIPE: printf("io_pipe"); break;
    case NODE_COMMAND_I_PIPE_O: printf("i_pipe_o"); break;
    case NODE_COMMAND_PIPE_IO: printf("pipe_io"); break;
    case NODE_COMMAND_SHELL: printf("shell"); break;
    case NODE_COMMAND_TYPE_SIZE: printf("type_size"); break;
    }
    if (node->type == NODE_COMMAND_TYPE_SIZE) { printf("}"); return; }
    if (node->type == NODE_COMMAND_SHELL) {
        printf(",command=");
        ast_print_node_shell(node->command);
        printf("}");
        return;
    }
    printf(",io_pipe=");
    ast_print_node_io_pipe(node->io_pipe);
    printf("}");
}

void ast_print_node_io_pipe(struct NodeIOPipe *node) {
    if (!node) { printf("nil"); return; }
    printf("NodeIOPipe{input=");
    if (node->input) printf("`%s`", node->input);
    else printf("nil");
    printf(",output=");
    if (node->output) printf("`%s`", node->output);
    else printf("nil");
    printf(",append=%d,pipe=", node->append);
    ast_print_node_pipeline(node->pipe);
    printf("}");
}

void ast_print_node_conditional(struct NodeConditional *node) {
    if (!node) { printf("nil"); return; }
    printf("NodeConditional{command=");
    ast_print_node_command(node->command);
    if (node->next) {
        printf(",if_false=%d,next=", node->if_false);
        ast_print_node_conditional(node->next);
    } else {
        printf(",next=nil");
    }
    printf("}");
}
