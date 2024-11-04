#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct Node
{
    struct Node *prev, *next;
    char *elem;
};

struct List
{
    struct Node *first, *last;
};

void move_to_end(struct List *pl, struct Node *node) {
    if (!pl || !node) return;
    if (!node->next) { assert(pl->last == node); return; }
    if (!node->prev) {
        node->next->prev = NULL;
        pl->first = node->next;
        node->next = NULL;
        node->prev = pl->last;
        pl->last->next = node;
        pl->last = node;
        return;
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = NULL;
    node->prev = pl->last;
    pl->last->next = node;
    pl->last = node;
}

void remove_node(struct List *pl, struct Node *node) {
    if (!pl || !node) return;
    if (!node->next && !node->prev) {
        assert(pl->first == node);
        assert(pl->last == node);
        pl->first = NULL;
        pl->last = NULL;
    } else if (!node->next) {
        assert(pl->last == node);
        node->prev->next = NULL;
        pl->last = node->prev;
        node->prev = NULL;
    } else if (!node->prev) {
        assert(pl->first == node);
        node->next->prev = NULL;
        pl->first = node->next;
        node->next = NULL;
    } else {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        node->prev = NULL;
        node->next = NULL;
    }
    free(node->elem);
    free(node);
}

void process(struct List *pl, const char *str) {
    if (!pl->first) return;
    struct Node *current = pl->first;
    struct Node *the_last = pl->last;
    while (current != the_last) {
        int diff = strcmp(current->elem, str);
        struct Node *next = current->next;
        if (!diff) remove_node(pl, current);
        else if (diff > 0) move_to_end(pl, current);
        current = next;
    }
    int diff = strcmp(the_last->elem, str);
    if (!diff) remove_node(pl, the_last);
    else if (diff > 0) move_to_end(pl, current);
}

int main(void) {
    {
        struct List x = {NULL, NULL};
        process(&x, "abc");
    }
    {
        struct Node *p = malloc(sizeof(*p)), *q = malloc(sizeof(*q)), *r = malloc(sizeof(*r));
        *p = (struct Node){
            .elem = "lol",
            .next = q,
            .prev = NULL,
        };
        *q = (struct Node){
            .elem = "keka",
            .next = r,
            .prev = p,
        };
        *r = (struct Node){
            .elem = "che",
            .next = NULL,
            .prev = q,
        };
        struct List x = {.first = p, .last = r};
        process(&x, "abc");
    }
    {
        struct Node *p = malloc(sizeof(*p)), *q = malloc(sizeof(*q)), *r = malloc(sizeof(*r));
        char *string = malloc(sizeof(char) * 4);
        strcpy(string, "abc");
        *p = (struct Node){
            .elem = string,
            .next = NULL,
            .prev = NULL,
        };
        // *q = (struct Node){
        //     .elem = string,
        //     .next = r,
        //     .prev = p,
        // };
        // *r = (struct Node){
        //     .elem = "che",
        //     .next = NULL,
        //     .prev = q,
        // };
        struct List x = {.first = p, .last = p};
        process(&x, "abc");
    }
    return 0;
}
