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

void ast_node_conditional_free(struct NodeConditional *node) {
    if (!node) return;
    if (node->commands) ast_node_pipeline_free(node->commands);
    if (node->next) ast_node_conditional_free(node->next);
    free(node);
}

void ast_node_pipeline_free(struct NodePipeline *node) {
    if (!node) return;
    if (node->pipe) ast_node_pipeline_free(node->pipe);
    switch (node->type) {
    case NODE_BASICITY_BASIC: ast_node_basic_command_free(node->command); break;
    case NODE_BASICITY_SHELL: ast_node_shell_free(node->shell); break;
    }
    free(node);
}

void ast_node_basic_command_free(struct NodeBasicCommand *node) {
    if (!node) return;
    free(node);
}

static void print_string_or_nil(char *string){
    if (!string) printf("nil");
    else printf("`%s`", string);
}

void ast_print_node_shell(struct NodeShell *node) {
    if (!node) { printf("nil"); return; }
    printf("NodeShell{new_shell=%d,bg=%d,input=", node->new_shell, node->bg);
    print_string_or_nil(node->input);
    printf(",output=");
    print_string_or_nil(node->output);
    printf(",append=%d,command=", node->append);
    ast_print_node_conditional(node->command);
    printf(",next=");
    ast_print_node_shell(node->next);
    printf("}");
}

void ast_print_node_conditional(struct NodeConditional *node) {
    if (!node) { printf("nil"); return; }
    printf("NodeConditional{commands=");
    ast_print_node_pipeline(node->commands);
    if (node->next) {
        printf(",next=");
        ast_print_node_conditional(node->next);
        printf(",if_false=%d", node->if_false);
    } else {
        printf(",next=nil");
    }
    printf("}");
}

void ast_print_node_pipeline(struct NodePipeline *node) {
    if (!node) { printf("nil"); return; }
    printf("NodePipeline{type=");
    switch (node->type) {
    case NODE_BASICITY_BASIC:
        printf("BASIC,command=");
        ast_print_node_basic_command(node->command);
        break;
    case NODE_BASICITY_SHELL:
        printf("SHELL,shell=");
        ast_print_node_shell(node->shell);
        break;
    }
    printf(",pipe=");
    ast_print_node_pipeline(node->pipe);
    printf("}");
}

void ast_print_node_basic_command(struct NodeBasicCommand *node) {
    if (!node) { printf("nil"); return; }
    printf("NodeBasicCommand{name=`%s`,args=[", node->name);
    for (char **p = node->args; *p; p++)
        printf("`%s`,", *p);
    printf("],input=");
    print_string_or_nil(node->input);
    printf(",output=");
    print_string_or_nil(node->output);
    printf(",append=%d}", node->append);
}
