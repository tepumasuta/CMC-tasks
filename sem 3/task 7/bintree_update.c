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

#define DIE(MSG) do { fputs(MSG, stderr); exit(1); } while (0)
#define DIE_SYS(MSG) do { perror(MSG); exit(1); } while (0)
#define WARN(MSG) fprintf(stderr, "[WARNING]: " MSG)
#define WARN_ARGS(...) do { fputs("[WARNING]: ", stderr); fprintf(stderr, __VA_ARGS__); } while (0)

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

off_t lseek_or_die(int fd, off_t offset, int whence);
int open_or_die(const char *filename, int flags, mode_t mode);

enum load_err {
    LOAD_OK = 0, EMPTY_FILE, SYS_FAIL, WRONG_FORMAT,
};

enum load_err tree_load(int fd, arena_t *arena, size_t *tree_size, node_t **tree);
bool tree_dump(const node_t *tree, int fd, const size_t *tree_size);
node_t *tree_create(kv_t element, arena_t *arena);
bool tree_push(node_t *tree, kv_t element, arena_t *arena);
void tree_print(const node_t *tree);
bool tree_print_from_file(int fd);
/* I laaack, i absolutely LAAAAAACK defer, why am i writing it not in Odin, just why... */
/* Also i lack explicitly, compiler checked nonnull pointers... */

bool read_pair(kv_t *kv);

static arena_t allocator = { .mem = NULL, .cap = 0, .at = 0 };
static int fd = -1;
static size_t tree_size = 0;
static node_t *tree = NULL;
static bool parsed_tree = false;
void cleanup(void) {
    if (parsed_tree) tree_dump(tree, fd, &tree_size);
    if (fd >= 0) { errno = 0; if (close(fd) < 0) perror("Failed to close the file"); }
    arena_free(&allocator);
}

int main(int argc, char *argv[]) {
    atexit(cleanup);
    const char *filename = process_arguments(argc, argv);
    if (!arena_init(&allocator)) DIE("[ERROR]: Failed to initialize arena, could not operate\n");
    fd = open_or_die(filename, O_RDWR | O_CREAT, 0666);
    switch (tree_load(fd, &allocator, &tree_size, &tree)) {
    case LOAD_OK: break;
    case EMPTY_FILE:
        WARN("File is empty\n");
        break;
    case SYS_FAIL: return 1;
    case WRONG_FORMAT: DIE("[ERROR]: Wrong file format\n");
    }
    parsed_tree = true;

    /* FOR DEBUG */
    // tree_print(tree);

    kv_t element;
    if (!tree) {
        if (!read_pair(&element)) {
            WARN("No <key,value> pairs received\n");
            return 0;
        }
        tree = tree_create(element, &allocator);
        if (!tree) DIE("[ERROR]: Failed to create tree\n");
    }

    while (read_pair(&element)) {
        if (!tree_push(tree, element, &allocator))
            WARN_ARGS("Failed to push pair <%d,%d>\n", element.key, element.value);
    }

    return 0;
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

static void usage(void) {
    puts("Usage: ./bintree_update <filename>");
}

#define DIE_USAGE(MSG) do { fputs(MSG, stderr); usage(); exit(1); } while (0)

const char *process_arguments(int argc, char *argv[]) {
    if (argc < 2) DIE_USAGE("");
    return argv[1];
}

off_t lseek_or_die(int fd, off_t offset, int whence) {
    errno = 0;
    off_t result = lseek(fd, offset, whence);
    if (result < 0) DIE_SYS("[ERROR]: Failed to lseek");
    return result;
}

int open_or_die(const char *filename, int flags, mode_t mode) {
    errno = 0;
    int result = open(filename, flags, mode);
    if (result < 0) DIE_SYS("[ERROR]: Failed to open");
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

static i32 tree_dump_into_rec(const node_t *tree, arena_t *tree_arena) {
    if (!tree) return 0;
    packed_node_t *packed_node;
    if (!(packed_node = arena_alloc(tree_arena, sizeof(packed_node_t)))) return false;
    i32 place = tree_arena->at / sizeof(packed_node_t) - 1;
    node_t *right = tree->right, *left = tree->left;
    *packed_node = (packed_node_t){
        .key = tree->key,
        .value = tree->value,
        .left = tree_dump_into_rec(left, tree_arena),
        .right = tree_dump_into_rec(right, tree_arena),
    };
    return place;
}

bool tree_dump(const node_t *tree, int fd, const size_t *tree_size) {
    arena_t tree_arena;
    if (!arena_init(&tree_arena)) return false;
    if (tree_size) if (!arena_reserve(&tree_arena, *tree_size + 1)) return false;
    packed_node_t *nil_node = arena_alloc(&tree_arena, sizeof(*nil_node));
    *nil_node = (packed_node_t){ 0 };
    if (tree && !tree_dump_into_rec(tree, &tree_arena)) {
        arena_free(&tree_arena);
        return false;
    }
    if (lseek(fd, 0, SEEK_SET) < 0) {
        arena_free(&tree_arena);
        return false;
    }
    errno = 0;
    if (write(fd, tree_arena.mem, tree_arena.at) < (ssize_t)tree_arena.at) {
        arena_free(&tree_arena);
        if (errno) perror("Failed to write");
        return false;
    }
    arena_free(&tree_arena);
    return true;
}

node_t *tree_create(kv_t element, arena_t *arena) {
    node_t *result = arena_alloc(arena, sizeof(*result));
    if (!result) return NULL;
    *result = (node_t){
        .key = element.key,
        .value = element.value,
        .left = NULL,
        .right = NULL,
    };
    return result;
}

bool tree_push(node_t *tree, kv_t element, arena_t *arena) {
    assert(tree);
    node_t *prev = NULL;
    bool right = false;
    while (tree && tree->key != element.key) {
        prev = tree;
        if (tree->key > element.key) {
            right = false;
            tree = tree->left;
        } else {
            right = true;
            tree = tree->right;
        }
    }
    if (!tree) {
        node_t *new_node = arena_alloc(arena, sizeof(*tree));
        if (!new_node) return false;
        *new_node = (node_t){
            .key = element.key,
            .value = element.value,
            .left = NULL,
            .right = NULL,
        };
        *(node_t**[2]){&prev->left, &prev->right}[right] = new_node;
        // (right ? prev->right : prev->left) = new_node;
        // if (right) prev->right = new_node; else prev->left = new_node;
    } else {
        tree->value = element.value;
    }
    return true;
}

bool read_pair(kv_t *kv) {
    return scanf("%d%d", &kv->key, &kv->value) == 2;
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
