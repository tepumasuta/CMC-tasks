#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define ALLOC_SIZE 64

void *try_alloc(size_t size, size_t *allocated) {
    void *mem = NULL;
    while ((mem = malloc(size)) == NULL && (size /= 2) >= 4);
    if (mem && allocated)
        *allocated = size;
    return mem;
}

void *try_realloc(void *old_mem, size_t size, size_t additional, size_t *reallocated) {
    void *mem = NULL;
    while ((mem = realloc(old_mem, size + additional)) == NULL && (additional /= 2) >= 4);
    if (mem && reallocated)
        *reallocated = size + additional;
    return mem;
}

char *read_string(bool *failed) {
    size_t cap, size = 0;
    char *buf = try_alloc(ALLOC_SIZE, &cap);
    if (!buf) {
        if (failed)
            *failed = true;
        return NULL;
    }

    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        if (size == cap - 1) {
            char *new_buf = try_realloc(buf, cap, ALLOC_SIZE, &cap);
            if (!new_buf) {
                if (failed)
                    *failed = true;
                buf[size] = '\0';
                return buf;
            }
	    buf = new_buf;
        }
        buf[size++] = c;
    }

    buf[size] = '\0';
    if (failed)
        *failed = false;
    return buf;
}


typedef struct string_node_t {
    char *string;
    struct string_node_t *prev, *next;
    bool owned;
} string_node_t;

typedef struct string_list_t {
    string_node_t *head, *end;
    size_t length;
} string_list_t;

void string_list_init(string_list_t *list) {
    if (!list) return;
    *list = (string_list_t){
        .head = NULL,
        .end = NULL,
        .length = 0,
    };
}

static inline void string_node_free(const string_node_t *node) {
    if (!node) return;
    if (node->owned)
        free(node->string);
    free(node);
}

void string_list_deinit(string_list_t *list) {
    if (!list) return;
    string_node_t *cur = list->head, *old;
    if(cur) while (cur->next) {
        old = cur;
        cur = cur->next;
        string_node_free(old);
    }
    string_node_free(cur);
    list->head = NULL;
    list->length = 0;
}

static string_node_t *string_node_from_string(char *string, bool own) {
    string_node_t *node = malloc(sizeof(*node));
    *node = (string_node_t){
        .string = string,
        .next = NULL,
        .prev = NULL,
        .owned = own,
    };
    return node;
}

void string_list_println(const string_list_t *list) {
    if (!list) {
        fputs("(* List<char *, 0>)nil", stdout);
        return;
    }
    printf("List<char *, %zu> {", list->length);
    if (!list->length) {
        puts("}");
        return;
    }
    string_node_t *head = list->head;
    if (head->string)
        printf(" nil <- `%s`", head->string);
    else
        fputs("nil", stdout);
    head = head->next;
    while (head) {
        if (head->string)
            printf(" <-> `%s`", head->string);
        else
            fputs(" <-> nil", stdout);
        head = head->next;
    }
    puts(" -> nil }");
}

static string_node_t *string_list_find(const string_list_t *list, size_t index) {
    if (!list || index > list->length || !index) return NULL;
    
    string_node_t *node;
    if (index < list->length / 2) {
        node = list->head;
        for (size_t i = 1; i < index; i++)
            node = node->next;
    } else {
        node = list->end;
        for (size_t i = list->length; i > index; i--)
            node = node->prev;
    }

    return node;
}

void string_list_remove(string_list_t *list, size_t index) {
    if (!list || index > list->length || !index) return;
    if (list->length == 1) {
        string_node_free(list->head);
        *list = (string_list_t){ 0 };
        return;
    }
    if (index == list->length) {
        string_node_t *tmp = list->end->prev;
        tmp->next = NULL;
        string_node_free(list->end);
        list->end = tmp;
        list->length--;
        return;
    }
    if (index == 1) {
        string_node_t *tmp = list->head->next;
        tmp->prev = NULL;
        string_node_free(list->head);
        list->head = tmp;
        list->length--;
        return;
    }

    string_node_t *node = string_list_find(list, index);
    node->next->prev = node->prev;
    node->prev->next = node->next;
    string_node_free(node);
    list->length--;
}

void string_list_insert(string_list_t *list, string_node_t *node, size_t index) {
    if (!list) return;
    if (index > list->length) index = list->length;
    if (!list->length) {
        list->head = list->end = node;
        list->length++;
        return;
    }
    if (!index) {
        list->head->prev = node;
        node->next = list->head;
        node->prev = NULL;
        list->head = node;
        list->length++;
        return;
    }
    if (index == list->length) {
        list->end->next = node;
        node->prev = list->end;
        node->next = NULL;
        list->end = node;
        list->length++;
        return;
    }
    string_node_t *before = string_list_find(list, index);
    node->next = before->next;
    node->prev = before;
    before->next->prev = node;
    before->next = node;
    list->length++;
}

static inline void skip_line(void) {
    int c;
    while ((c = getchar()) != EOF && c != '\n');
}

void string_list_remove_from_stdin(string_list_t *list) {
    size_t index;
    fputs("\nInput index to be removed: ", stdout);
    while (!feof(stdin) && scanf("%zu", &index) != 1) {
        skip_line();
        fputs("\nInput index to be removed: ", stdout);
    }
    skip_line();    
    if (feof(stdin)) return;
    string_list_remove(list, index);
}

void string_list_insert_stdin(string_list_t *list) {
    size_t index;
    fputs("\nInput index to inserted after: ", stdout);
    while (!feof(stdin) && scanf("%zu", &index) != 1) {
        skip_line();
        fputs("Input index to inserted after: ", stdout);
    }
    skip_line();
    fputs("Input string: ", stdout);
    bool failed;
    char *string = read_string(&failed);
    if (feof(stdin)) return;
    string_list_insert(list, string_node_from_string(string, true), index);
}

static void (*cycled_functions[])(string_list_t *) = {
    string_list_println,
    string_list_remove_from_stdin,
    string_list_insert_stdin,
};

#define ARR_LEN(ARR) (sizeof(ARR)/sizeof(*ARR))


/* I could cycle functions but that barely makes any sense because
   each cycle 1 node is inserted and at most 1 node is removed, so
   the list only grows or stays the same */
int main(void) {
    string_list_t list;
    string_list_init(&list);
    while (!feof(stdin)) {
        puts("Input operation (r/p/i): ");
        int c;
        while ((c = getchar()) == '\n');

        size_t i;
        switch (c) {
        case 'p': i = 0; break;
        case 'r': i = 1; break;
        case 'i': i = 2; break;
        case 'q': goto end;
        default: continue;
        }
        cycled_functions[i](&list);
    }
end:
    string_list_deinit(&list);
    return 0;
}
