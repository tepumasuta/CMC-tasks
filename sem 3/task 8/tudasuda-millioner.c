#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef __linux
#include <linux/limits.h>
#elif __unix
#include <sys/limits.h>
#endif

#define S1(x) #x
#define S2(x) S1(x)

#define return_defer(RET_VAL) do { result = (RET_VAL); goto defer; } while(0)
#define DIE_USAGE(MSG) do { fprintf(stderr, "[FATAL:"S2(__LINE__)"]: " MSG "\n"); usage(); return_defer(1); } while (0)
#define DIE(MSG) do { fprintf(stderr, "[FATAL:"S2(__LINE__)"]: " MSG "\n"); return_defer(1); } while (0)
#define DIE_SYS(MSG) do { perror("[FATAL:"S2(__LINE__)"]: " MSG); return_defer(1); } while(0)
#define ERR_SYS(MSG) do { perror("[ERROR:"S2(__LINE__)"]: " MSG); } while(0)

#ifndef PIPE_BUF
#define PIPE_BUF 4096
#endif

#define LINE_BUF_SIZE 2048
static_assert(LINE_BUF_SIZE <= PIPE_BUF);

#define READ 0
#define WRITE 1

void usage(void);
void reverse(char *from, char *to);
void reverse_words(char *string, size_t length);
int min(int x, int y);

int main(int argc, char *argv[]) {
    /* Deferred section */
    FILE *file = NULL;
    char *string = NULL;
    int rw_ptc[2] = { -1, -1 }; /* parent to child */
    int rw_ctp[2] = { -1, -1 }; /* child to parent */

    int result = 0;

    if (argc < 2) DIE_USAGE("Too few arguments, filename required");

    if (pipe(rw_ptc) < 0) DIE_SYS("Failed to open pipe");
    if (pipe(rw_ctp) < 0) DIE_SYS("Failed to open pipe");

    pid_t p = fork();
    if (p < 0) DIE_SYS("Failed to fork");
    else if (!p) {
        if (close(rw_ctp[READ]) < 0) DIE_SYS("Failed to close pipe");
        rw_ctp[READ] = -1;
        if (close(rw_ptc[WRITE]) < 0) DIE_SYS("Failed to close pipe");
        rw_ptc[WRITE] = -1;

        size_t cap = LINE_BUF_SIZE + 1;
        string = malloc(sizeof(*string) * cap);
        if (!string) DIE("Failed to malloc");

        size_t at = 0;
        ssize_t actually_read;
        while ((actually_read = read(rw_ptc[READ], string + at, LINE_BUF_SIZE)) > 0) {
            at += actually_read;
            if (string[at - 1] == '\n') {
                string[at] = '\0';
                reverse_words(string, at);

                size_t write_at  = 0;
                size_t write_size = LINE_BUF_SIZE;
                while (write_at < at) {
                    if (write_at + write_size > at - write_at) write_size = at - write_at;
                    if (write(rw_ctp[WRITE], string + write_at, write_size) < (ssize_t)write_size) DIE_SYS("Failed to write");
                    write_at += write_size;
                }

                at = 0;
            }
            if (at >= cap - LINE_BUF_SIZE - 1) {
                cap *= 2;
                void *new_mem = realloc(string, sizeof(*string) * cap);
                if (!new_mem) DIE("Failed to realloc");
                string = new_mem;
            }
        }
        if (actually_read < 0) DIE_SYS("Failed to read");
        if (at) {
            string[at] = '\0';
            reverse_words(string, at);

            size_t write_at = 0;
            size_t write_size = LINE_BUF_SIZE;
            while (write_at < at) {
                if (write_at + write_size > at - write_at) write_size = at - write_at;
                if (write(rw_ctp[WRITE], string + write_at, write_size) < (ssize_t)write_size) DIE_SYS("Failed to write");
                write_at += write_size;
            }
        }

        free(string);
        string = NULL;
        if (close(rw_ctp[WRITE]) < 0) DIE_SYS("Failed to close pipe");
        rw_ctp[WRITE] = -1;
        if (close(rw_ptc[READ]) < 0) DIE_SYS("Failed to close pipe");
        rw_ptc[READ] = -1;
    } else {
        file = fopen(argv[1], "r+");
        if (!file) DIE_SYS("Failed to open file");

        if (close(rw_ctp[WRITE]) < 0) DIE_SYS("Failed to close pipe");
        rw_ctp[WRITE] = -1;
        if (close(rw_ptc[READ]) < 0) DIE_SYS("Failed to close pipe");
        rw_ptc[READ] = -1;
        
        char buf[LINE_BUF_SIZE + 1] = { 0 };
        while (fgets(buf, LINE_BUF_SIZE + 1, file)) {
            /* Write string to child */
            size_t buf_size = strlen(buf);
            size_t length = buf_size;
            bool read_fully = buf_size != LINE_BUF_SIZE || buf[LINE_BUF_SIZE] == '\n';
            bool read_performed = true;
            while (!read_fully) {
                if (write(rw_ptc[WRITE], buf, buf_size) < (ssize_t)buf_size) DIE_SYS("Failed to write");
                if (!(read_performed = fgets(buf, LINE_BUF_SIZE + 1, file))) break;
                buf_size = strlen(buf);
                length += buf_size;
                read_fully = buf_size != LINE_BUF_SIZE || buf[LINE_BUF_SIZE] == '\n';
            }
            if (read_performed) if (write(rw_ptc[WRITE], buf, buf_size) < (ssize_t)buf_size) DIE_SYS("Failed to write");
            if (!read_performed || buf[buf_size - 1] != '\n') {
                if (close(rw_ptc[WRITE]) < 0) DIE_SYS("Failed to close pipe");
                rw_ptc[WRITE] = -1;
            }

            /* Receive string from child and write back */
            ssize_t left = length;
            if (fseek(file, -length, SEEK_CUR) < 0) DIE_SYS("Failed to fseek");
            ssize_t actually_read;
            while (left && (actually_read = read(rw_ctp[READ], buf, min(LINE_BUF_SIZE, left))) > 0) {
                left -= actually_read;
                if (fwrite(buf, 1, actually_read, file) < (size_t)actually_read) DIE_SYS("Failed to fwrite");
            }
            if (actually_read < 0) DIE_SYS("Failed to read");
        }
        if (close(rw_ctp[READ]) < 0) DIE_SYS("Failed to close pipe");
        rw_ctp[READ] = -1;
        if (rw_ptc[WRITE] >= 0) if (close(rw_ptc[WRITE]) < 0) DIE_SYS("Failed to close pipe");
        rw_ptc[WRITE] = -1;
        if (fclose(file)) DIE_SYS("Failed to close file");
        file = NULL;
    }

defer:
    if (file) if (fclose(file)) ERR_SYS("Failed to close file");
    if (string) free(string);
    for (size_t i = 0; i < 2; i++) if (rw_ptc[i] >= 0) if (close(rw_ptc[i]) < 0) ERR_SYS("Failed to close");
    for (size_t i = 0; i < 2; i++) if (rw_ctp[i] >= 0) if (close(rw_ctp[i]) < 0) ERR_SYS("Failed to close");
    return result;
}

void reverse(char *from, char *to) {
    assert(*to != '\0');
    while (from < to) {
        char tmp = *from;
        *from++ = *to;
        *to-- = tmp;
    }
}

/* Well, it's not optimal, but it works and it's easy to write and ponder about.
   And i guess that part is not the main one, so I won't care about efficiency here for now. */
void reverse_words(char *string, size_t length) {
    size_t the_length = length;
    if (string[length - 1] == '\n') {
        string[length - 1] = '\0';
        the_length--;
    }
    if (the_length) {
        reverse(string, string + the_length - 1);
        char *token = strtok(string, " \t");
        do {
            reverse(token, token + strlen(token) - 1);
        } while ((token = strtok(NULL, " \t")));
        for (size_t i = 0; i < the_length; i++)
            if (string[i] == '\0')
                string[i] = ' '; /* [BUG]: Would change \t to ' ' */
    }
    if (the_length < length) {
        string[length - 1] = '\n';
    }
}

int min(int x, int y) {
    return x < y ? x : y;
}

void usage(void) {
    puts("Usage: ./tudasuda-millioner <filename>");
}
