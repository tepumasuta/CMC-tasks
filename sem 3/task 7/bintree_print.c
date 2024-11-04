#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#define DIE_SYS(MSG) do { perror(MSG); exit(1); } while (0)
#define DIE(MSG) do { fputs(MSG, stderr); exit(1); } while (0)

#define DEFAULT_ARENA_SIZE 1024

typedef int32_t i32;

typedef struct {
    i32 key, value;
} kv_t;

typedef struct {
    i32 key, value, left, right;
} packed_node_t;

typedef struct _node {
    i32 key, value;
    struct _node *left, *right;
} node_t;

_Static_assert(sizeof(packed_node_t) == 4 * 4);

typedef struct {
    void *mem;
    size_t cap, at;
} arena_t;

bool arena_init(arena_t *arena);
void arena_free(arena_t *arena);
void *arena_alloc(arena_t *arena, size_t size /* in bytes */);
bool arena_reserve(arena_t *arena, size_t size /* in bytes */);

/* Exits on fail */
const char *process_arguments(int argc, char *argv[]);

int open_or_die(const char *filename, int flags, mode_t mode);

enum load_err {
    LOAD_OK = 0, EMPTY_FILE, SYS_FAIL, WRONG_FORMAT,
};

enum load_err tree_load(int fd, arena_t *arena, size_t *tree_size, node_t **tree);
void tree_print(const node_t *tree);
bool tree_print_from_file(int fd);
/* I laaack, i absolutely LAAAAAACK defer, why am i writing it not in Odin, just why... */
/* Also i lack explicitly, compiler checked nonnull pointers... */

static int fd = -1;
void cleanup(void) {
    if (fd >= 0) { errno = 0; if (close(fd) < 0) perror("Failed to close the file"); }
}

int main(int argc, char *argv[]) {
    atexit(cleanup);
    const char *filename = process_arguments(argc, argv);
    fd = open_or_die(filename, O_RDONLY, 0666);

    return !tree_print_from_file(fd);
}

bool tree_print_from_file(int fd) {
    arena_t allocator;
    if (!arena_init(&allocator)) {
        fputs("[ERROR]: Failed to initialize arena\n", stderr);
        return false;
    }
    node_t *tree = NULL;
    switch (tree_load(fd, &allocator, NULL, &tree)) {
    case LOAD_OK: break;
    case EMPTY_FILE:
        puts("The file is empty, wrong format");
        return 0;
    case SYS_FAIL: return 1;
    case WRONG_FORMAT: DIE("[ERROR]: Wrong file format\n");
    }

    tree_print(tree);

    arena_free(&allocator);
    return true;
}

static void tree_print_rec(const node_t *tree) {
    if (!tree) return;
    tree_print_rec(tree->left);
    printf("Node{key=%d,value=%d,left=%d,right=%d},", tree->key, tree->value,
        tree->left ? tree->left->key : 0, tree->right ? tree->right->key : 0);
    tree_print_rec(tree->right);
}

void tree_print(const node_t *tree) {
    if (!tree) {
        puts("(Tree *)nil");
        return;
    }
    fputs("Tree {", stdout);
    tree_print_rec(tree);
    puts("}");
}

static void usage(void) {
    puts("Usage: ./bintree_print <filename>\n");
}

#define DIE_USAGE(MSG) do { fputs(MSG, stderr); usage(); exit(1); } while (0)

const char *process_arguments(int argc, char *argv[]) {
    if (argc < 2) DIE_USAGE("");
    return argv[1];
}

int open_or_die(const char *filename, int flags, mode_t mode) {
    errno = 0;
    int result = open(filename, flags, mode);
    if (result < 0) DIE_SYS("[ERROR]: Failed to open");
    return result;
}

bool arena_init(arena_t *arena) {
    assert(arena);
    *arena = (arena_t){
        .mem = malloc(DEFAULT_ARENA_SIZE),
        .cap = DEFAULT_ARENA_SIZE,
        .at = 0,
    };
    if (!arena->mem) {
        arena->cap = 0;
        return false;
    }
    return true;
}

void arena_free(arena_t *arena) {
    assert(arena);
    if (!arena->mem) return;
    free(arena->mem);
}

void *arena_alloc(arena_t *arena, size_t size /* in bytes */) {
    assert(arena);
    if (arena->cap - arena->at + 1 < size)
        if (!arena_reserve(arena, arena->cap * 2)) return NULL;
    void *result = arena->mem + arena->at;
    arena->at += size;
    return result;
}

static node_t *build_tree(arena_t *arena, packed_node_t *base, size_t at) {
    node_t *cur = arena_alloc(arena, sizeof(*cur));
    if (!cur) exit(1); /* Memory should be reserved before call */
    *cur = (node_t){
        .key = base[at].key,
        .value = base[at].value,
        .left = base[at].left ? build_tree(arena, base, base[at].left) : NULL,
        .right = base[at].right ? build_tree(arena, base, base[at].right) : NULL,
    };
    return cur;
}

enum load_err tree_load(int fd, arena_t *arena, size_t *tree_size, node_t **tree) {
    assert(tree);
    const ssize_t filesize = lseek(fd, 0, SEEK_END);
    if (filesize < 0) {
        perror("Failed to lseek");
        return SYS_FAIL;
    }
    if (!filesize) return EMPTY_FILE;
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("Failed to lseek");
        return SYS_FAIL;
    }
    const size_t total_nodes = filesize / sizeof(packed_node_t);
    packed_node_t *packed_tree = malloc(sizeof(*packed_tree) * total_nodes);
    if (!packed_tree) return false;
    if (read(fd, packed_tree, filesize) < filesize) {
        free(packed_tree);
        return SYS_FAIL;
    }
    if (memcmp(&packed_tree[0], &(packed_node_t){ 0 }, sizeof(packed_node_t))) {
        free(packed_tree);
        return WRONG_FORMAT;
    }
    if (!arena_reserve(arena, sizeof(node_t) * total_nodes + arena->cap)) {
        free(packed_tree);
        return SYS_FAIL;
    }
    if (total_nodes > 1 && !(*tree = build_tree(arena, packed_tree, 1))) {
        free(packed_tree);
        return WRONG_FORMAT;
    }

    free(packed_tree);
    if (tree_size) *tree_size = total_nodes - 1;
    return LOAD_OK;
}

bool arena_reserve(arena_t *arena, size_t size /* in bytes */) {
    assert(arena);
    if (arena->cap < size) {
        void *new_mem = realloc(arena->mem, size);
        if (!new_mem) return false;
        arena->mem = new_mem;
        arena->cap = size;
    }
    return true;
}
