#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SMALL_BUF_CAP 2048

#define DIE(MSG) do { perror(MSG); exit(1); } while (0)

typedef struct {
    char *buf, *cp;
    const char *ep;
    size_t cap;
} buffer_t;

typedef struct {
    char *buf;
    size_t at, cap;
} linebuf_t;

void usage(void);
void process_arguments(int argc, char *argv[], size_t *length, const char **filename);
/* Returns pointer to mallocated memory
   I wish libc simply had arenas... */
void *create_iow_buf_pair(buffer_t *const in, buffer_t *const out, const size_t buf_cap,
                          linebuf_t *const window, const size_t window_length);
void usage(void);
void process_arguments(int argc, char *argv[], size_t *length, const char **filename);
size_t read_line_into(const int fd, buffer_t *const from, linebuf_t *const into);
size_t try_write(buffer_t *const from, const int fd, const off_t offset);
void perform_split(const size_t linelength);
void perform_full_read(const ssize_t filelength, const size_t linelength);

static int fd = -1;
static char *buffer = NULL;
void cleanup(void) {
    if (fd >= 0) if (close(fd) < 0) perror("[ERROR]: Failed to close the file\n");
    if (buffer) free(buffer);
}


int main(int argc, char *argv[]) {
    size_t linelength;
    const char *filename;

    process_arguments(argc, argv, &linelength, &filename);

    fd = open(filename, O_RDWR);
    if (fd < 0) DIE_SYS("[ERROR]: Failed to open file");
    atexit(cleanup);

    const ssize_t filelength = lseek(fd, 0, SEEK_END);

#ifndef ASSUME_MEM_LIMIT
    buffer = malloc(sizeof(*buffer) * filelength);
#endif
    if (!buffer) perform_split(linelength);
    else perform_full_read(filelength, linelength);

    return 0;
}

void usage(void) {
    puts("Usage: ./shrink-bigdata <filename> <max-line-length>");
}

void process_arguments(int argc, char *argv[], size_t *length, const char **filename) {
    if (argc < 3) {
        fputs("[ERROR]: Too few arguments\n", stderr);
        usage();
        exit(1);
    }

    errno = 0;
    int file_length = atoi(argv[2]); 
    if (errno) {
        perror("[ERROR]: Failed to convert to int\n");
        usage();
        exit(1);
    }
    if (file_length < 0) {
        fputs("[ERROR]: Max line length has to be >= 0\n", stderr);
        exit(1);
    }
    *length = file_length;
    *filename = argv[1];
}

void *create_iow_buf_pair(buffer_t *const in, buffer_t *const out, const size_t buf_cap,
                          linebuf_t *const window, const size_t window_length) {
    if (!in || !out || !window) return NULL;
    void *const mem = malloc(sizeof(*in->buf) * buf_cap * 2 + sizeof(*window->buf) * window_length);
    if (!mem) return NULL;
    *in = (buffer_t){
        .buf = mem,
        .ep = mem + buf_cap,
        .cp = mem + buf_cap,
        .cap = buf_cap,
    };
    *out = (buffer_t){
        .buf = (char *)in->ep,
        .ep = in->ep + buf_cap,
        .cp = (char *)in->ep,
        .cap = buf_cap,
    };
    *window = (linebuf_t){
        .buf = (char *)out->ep,
        .cap = window_length,
        .at = 0,
    };
    return mem;
}

static size_t try_read(buffer_t *const into, const int fd) {
    const ssize_t actually_read = read(fd, into->buf, into->cap);
    if (actually_read < 0) DIE_SYS("[ERROR]: Failed to read");
    into->cp = into->buf;
    if ((size_t)actually_read < into->cap)
        into->ep = into->buf + actually_read;
    return actually_read;
}

static size_t try_build_up(const char **const from, const char *const to, linebuf_t *into) {
    const size_t line_part_length = to - *from;
    if (line_part_length <= into->cap - into->at) {
        memcpy(into->buf + into->at, *from, line_part_length);
        into->at += line_part_length;
    }
    *from += line_part_length;
    return line_part_length;
}

/* Returns line length */
size_t read_line_into(const int fd, buffer_t *const from, linebuf_t *const into) {
    into->at = 0;
    size_t line_length = 0;
    while (1) {
        const char *const endline = memchr(from->cp, '\n', from->ep - from->cp);
        if (!endline) {
            line_length += try_build_up((const char **)&from->cp, from->ep, into);
            const size_t read_size = try_read(from, fd);
            if (!read_size) break; /* EOF */
            continue;
        }
        line_length += try_build_up((const char **)&from->cp, endline + 1, into);
        return line_length;
    }
    line_length += try_build_up((const char **)&from->cp, from->ep, into);
    return line_length;
}

size_t try_write(buffer_t *const from, const int fd, const off_t offset) {
    const ssize_t block_size = from->cp - from->buf;
    if (pwrite(fd, from->buf, block_size, offset) < block_size)
        DIE_SYS("[ERROR]: Failed to write");
    from->cp = from->buf;
    return block_size;
}

void write_line_into(const int fd, off_t *const offset, buffer_t *const to,
                     const linebuf_t *const from) {
    const char *cfp = from->buf;
    size_t left = from->at;
    while (left) {
        if (left < (size_t)(to->ep - to->cp)) {
            memcpy(to->cp, cfp, left);
            to->cp += left;
            return;
        }
        memcpy(to->cp, cfp, to->ep - to->cp);
        cfp += to->ep - to->cp;
        left -= to->ep - to->cp;
        to->cp = (char *)to->ep;
        *offset += try_write(to, fd, *offset);
    }
}

void perform_split(const size_t linelength) {
    buffer_t read_buf, write_buf;
    linebuf_t view;
    const size_t view_length = linelength + 1;
    buffer = create_iow_buf_pair(&read_buf, &write_buf, SMALL_BUF_CAP, &view, view_length);
    if (!buffer) buffer = create_iow_buf_pair(&read_buf, &write_buf, view_length, &view, view_length);
    if (!buffer) DIE_SYS("[ERROR]: Failed to malloc unable to operate");

    off_t last_write = lseek(fd, 0, SEEK_SET);
    size_t read_length;
    while ((read_length = read_line_into(fd, &read_buf, &view))) {
        if (read_length <= linelength || read_length == linelength + 1 && view.buf[linelength] == '\n') {
            write_line_into(fd, &last_write, &write_buf, &view);
        }
    }
    last_write += try_write(&write_buf, fd, last_write);

    if (ftruncate(fd, last_write - lseek(fd, 0, SEEK_SET)) < 0)
        DIE_SYS("[ERROR]: Failed to truncate file");
}

void perform_full_read(const ssize_t filelength, const size_t linelength) {
    lseek(fd, 0, SEEK_SET);
    errno = 0; /* It broke on 4GB file, so I had to manually set and check errno */
    if (read(fd, buffer, filelength) < filelength && errno != 0)
        DIE_SYS("[ERROR]: Failed to read from file");

    char *wp = buffer;
    const char *cp = buffer;
    const char *const ep = &buffer[filelength];
    bool exceeded = false;
    while (cp < ep) {
        // Read line
        const char *lp = cp;
        while (lp < ep && *lp != '\n') lp++; /* Maybe replace with memchr */
        const size_t cur_linelength = lp - cp;
        if (cur_linelength <= linelength) {
            if (!exceeded) {
                cp = wp = lp + 1;
                continue;
            }
            while (cp <= lp && cp < ep) /* Maybe replace with memmove */
                *wp++ = *cp++;
        } else {
            cp = lp + 1;
            exceeded = true;
        }
    }

    const ssize_t new_filelength = wp - buffer;
    if (ftruncate(fd, new_filelength) < 0) DIE_SYS("[ERROR]: Failed to truncate file");
    lseek(fd, 0, SEEK_SET);
    if (write(fd, buffer, new_filelength) < new_filelength) DIE_SYS("[ERROR]: Failed to write back to file");
}
