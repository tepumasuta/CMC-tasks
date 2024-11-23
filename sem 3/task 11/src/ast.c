#include "ast.h"

#include <stdlib.h>
#include <assert.h>

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
    assert(node->type != NODE_TYPE_SIZE);
    if (node->type == NODE_COMMAND_SHELL)
        if (node->command) ast_node_shell_free(node->command);
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
