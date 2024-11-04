#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

#define DEFAULT_BUF_SIZE 2048

#define DIE_SYS_ARGS(...) do { fprintf(stderr, __VA_ARGS__); perror(" "); exit(1); } while (0)
#define DIE_SYS(MSG) do { perror(MSG); exit(1); } while (0)
#define DIE(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)
#define DIE_USE(...) do { fprintf(stderr, __VA_ARGS__); usage(); exit(1); } while (0)

typedef enum {
    OUTPUT_ALL = 0, OUTPUT_UNIQUE, OUTPUT_REPEAT, OUTPUT_COUNT
} output_t;

typedef struct {
    bool print_reps_count;
    output_t print;
    size_t skip_first;
} options_t;

typedef struct {
    char *buf;
    size_t cap;
} buf_t;

typedef struct {
    buf_t view;
    size_t index;
    off_t base;
    size_t fullness;
} search_view_t;

typedef struct {
    off_t from, to;
} span_t;

void usage(void);
options_t parse_options(const int argc, const char *const argv[], const char **const filename);
int open_file(const char *filename);
void print_lines(const int fd, const options_t options);

static int fd = -1;
static void *buffs = NULL;
void cleanup(void) {
    if (fd >= 0) if (close(fd) < 0) perror("[ERROR]: Failed to close the file");
    if (buffs) free(buffs);
}

int main(int argc, char *argv[]) {
    atexit(cleanup);

    const char *filename;
    options_t options = parse_options(argc, argv, &filename);
    fd = open_file(filename);
    print_lines(fd, options);

    return 0;
}

static bool parse_num(const char *string, size_t *const num) {
    if (!string || !num) return false;
    size_t result = 0;
    while (*string) {
        if (*string > '9' || *string < '0') return false;
        result = result * 10 + *string - '0';
        string++;
    }
    *num = result;
    return true;
}

options_t parse_options(const int argc, const char *const argv[], const char **const filename) {
    assert(filename);
    options_t options = {
        .skip_first = 0,
        .print_reps_count = false,
        .print = OUTPUT_ALL,
    };

    size_t i = 1;
    bool parsed_skip = false;
    while (i < argc - 1) {
        if (argv[i][0] != '-' || strlen(argv[i]) != 2)
            DIE_USE("[ERROR]: Failed to parse options, unknown options `%s`\n", argv[i]);

        switch (argv[i][1]) {
        case 'c':
            if (options.print_reps_count) fputs("[WARNING]: `-c` option passed twice\n", stderr);
            options.print_reps_count = true;
            i++;
            break;
        case 'd':
            switch (options.print) {
            case OUTPUT_ALL: options.print = OUTPUT_REPEAT; break;
            case OUTPUT_REPEAT: fputs("[WARNING]: `-d` option passed twice\n", stderr); break;
            case OUTPUT_UNIQUE: DIE_SYS("[ERROR]: Can't mix options `-d` & `-u`\n"); break;
            case OUTPUT_COUNT: exit(2);
            }
            i++;
            break;
        case 'u':
            switch (options.print) {
            case OUTPUT_ALL: options.print = OUTPUT_UNIQUE; break;
            case OUTPUT_REPEAT: DIE_SYS("[ERROR]: Can't mix options `-d` & `-u`"); break;
            case OUTPUT_UNIQUE: fputs("[WARNING]: `-c` option passed twice\n", stderr); break;
            case OUTPUT_COUNT: exit(2);
            }
            i++;
            break;
        case 's':
            i++;
            if (!parse_num(argv[i], &options.skip_first))
                DIE_USE("[ERROR]: Failed to parse number `%s`\n", argv[i]);
            if (parsed_skip) fputs("[WARNING]: `-s` option passed twice\n", stderr);
            parsed_skip = true;
            i++;
            break;
        default:
            DIE_USE("[ERROR]: Failed to parse options, unknown options `%s`\n", argv[i]);
        }
    }

    *filename = argv[i];

    return options;
}

int open_file(const char *const filename) {
    const int fd = open(filename, O_RDONLY);
    if (fd < 0) DIE_SYS_ARGS("[ERROR]: Failed to open file `%s`", filename);
    return fd;
}

static bool setup_buffers(buf_t *const cur, buf_t *const fur, buf_t *const search_view_buf, size_t capacity) {
    buffs = malloc(sizeof(char) * capacity * 3);
    if (!buffs) return false;
    *cur = (buf_t){
        .buf = buffs,
        .cap = capacity,
    };
    *fur = (buf_t){
        .buf = buffs + capacity,
        .cap = capacity,
    };
    *search_view_buf = (buf_t){
        .buf = buffs + capacity * 2,
        .cap = capacity,
    };
    return true;
}

static bool next_line_span(const int fd, search_view_t *const view, span_t *const span) {
    span->from = view->base + view->index;
    const char *at;
    while (!(at = memchr(view->view.buf + view->index, '\n', view->fullness - view->index))) {
        ssize_t read_size = read(fd, view->view.buf, view->view.cap);
        if (read_size == 0) {
            span->to = view->base + view->fullness - 1;
            view->index = view->fullness - 1;
            return span->to > span->from;
        }
        view->base += view->view.cap;
        view->index = 0;
        if (read_size < 0) DIE_SYS("[ERROR]: Failed to read\n");
        if (read_size < view->view.cap) view->fullness = read_size;
    }
    view->index = at - view->view.buf + 1;
    span->to = view->base + view->index - 1;
    return true;
}

static ssize_t span_len(const span_t span) {
    return span.to - span.from + 1;
}

static bool span_at_end(const int fd, const span_t span) {
    char c;
    if (pread(fd, &c, sizeof(c), span.to) < sizeof(c)) DIE_SYS("Failed to read\n");
    return c != '\n';
}

static bool span_equal(const int fd, buf_t *const b1, buf_t *const b2, const span_t s1, const span_t s2, const size_t skip_first) {
    if (span_len(s1) < span_len(s2)) return span_equal(fd, b1, b2, s2, s1, skip_first);
    if (span_len(s1) <= skip_first + 1) return true;
    const size_t length = span_at_end(fd, s2)
                        ? span_len(s1) - 1
                        : span_len(s1);
    if (length != span_len(s2)) return false;
    size_t read_size = b1->cap;
    size_t left = length - skip_first;
    while (left) {
        if (left <= b1->cap) read_size = left;
        if (pread(fd, b1->buf, read_size, s1.from + length - left) < read_size) DIE_SYS("Failed to read\n");
        if (pread(fd, b2->buf, read_size, s2.from + length - left) < read_size) DIE_SYS("Failed to read\n");
        if (memcmp(b1->buf, b2->buf, read_size)) return false;
        left -= read_size;
    }
    return true;
}

static void print_line(const int fd, buf_t *const line_buf, const span_t span) {
    const size_t length = span_len(span);
    size_t read_size = line_buf->cap;
    size_t left = length;
    while (left) {
        if (left <= line_buf->cap) read_size = left;
        if (pread(fd, line_buf->buf, read_size, span.from + length - left) < read_size) DIE_SYS("Failed to read\n");
        printf("%.*s", (int)read_size, line_buf->buf);
        left -= read_size;
    }
}

void print_lines(const int fd, const options_t options) {
    buf_t current_line, further_line;
    search_view_t search_view = { .index = 0, .base = 0, .fullness = 0, };
    if (!setup_buffers(&current_line, &further_line, &search_view.view, DEFAULT_BUF_SIZE))
    if (!setup_buffers(&current_line, &further_line, &search_view.view, 1))
        DIE_SYS("[ERROR]: Failed to allocate memory\n");
    const ssize_t read_size = read(fd, search_view.view.buf, search_view.view.cap);
    if (read_size < 0) DIE_SYS("[ERROR]: Failed to read\n");
    search_view.fullness = read_size;

    // off_t cur_idx, cur_end, fur_idx, fur_end;
    span_t cur, fur;
    if (!next_line_span(fd, &search_view, &cur)) return;
    while (1) {
        switch (options.print) {
        case OUTPUT_ALL:
            size_t count = 1;
            bool eof_fail;
            while ((eof_fail = next_line_span(fd, &search_view, &fur))) {
                if (!span_equal(fd, &current_line, &further_line, cur, fur, options.skip_first)) break;
                count++;
            }
            if (options.print_reps_count) printf("\t%lu ", count);
            print_line(fd, &current_line, cur);
            if (!eof_fail) return;
            cur = fur;
            break;
        case OUTPUT_UNIQUE:
            if (!next_line_span(fd, &search_view, &fur)) {
                if (options.print_reps_count) fputs("\t1 ", stdout);
                print_line(fd, &current_line, cur);
                return;
            }
            if (!span_equal(fd, &current_line, &further_line, cur, fur, options.skip_first)) {
                if (options.print_reps_count) fputs("\t1 ", stdout);
                print_line(fd, &current_line, cur);
            } else {
                bool eof_fail;
                while ((eof_fail = next_line_span(fd, &search_view, &fur))
                    && span_equal(fd, &current_line, &further_line, cur, fur, options.skip_first));
                if (!eof_fail) return;
            }
            cur = fur;
            break;
        case OUTPUT_REPEAT:
            if (!next_line_span(fd, &search_view, &fur)) return;
            if (span_equal(fd, &current_line, &further_line, cur, fur, options.skip_first)) {
                size_t count = 2;
                while (next_line_span(fd, &search_view, &fur)
                  && span_equal(fd, &current_line, &further_line, cur, fur, options.skip_first))
                    count++;
                if (options.print_reps_count) printf("\t%lu ", count);
                print_line(fd, &current_line, cur);
            }
            cur = fur;
            break;
        case OUTPUT_COUNT: abort();
        }
    }
}

void usage(void) {
    puts("Usage: ./uniqum [-c] [-d] [-u] [-s <num>] <filename>\n"
         "    -c – перед каждой выходной строкой помещать ее кратность во входном файле\n"
         "    -d - подавить вывод неповторяющихся строк\n"
         "    -u - выводить только уникальные строки\n"
         "    -s <num> - при сравнении игнорировать заданное число <num> начальных символов строки");
}
