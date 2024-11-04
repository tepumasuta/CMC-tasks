// #define __USE_XOPEN2K8 1

#include <linux/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <linux/limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define DIR_BUF_SIZE PATH_MAX
#define FDS_BUF_SIZE PATH_MAX

typedef struct {
    char *buf;
    size_t idx, cap;
} charbuf_t;

void buf_init(charbuf_t *buf, size_t cap);
void buf_destory(charbuf_t *buf);
bool buf_push_path(charbuf_t *buf, const char *path);
char *buf_pop_path(charbuf_t *buf);

static const char *const months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};
static const char *const days[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

typedef void *(*visitor_fn(struct stat *, const char *))(struct stat *, const char *);

/* Has to be free-ed */
char *convert_to_readable(time_t time);
void process_directories(DIR *dir, charbuf_t *path_buf, visitor_fn *visitor);
bool push_dir(DIR *dir);
DIR *pop_dir(void);
bool push_fd(int fd);
int pop_fd(void);
visitor_fn first_layer;
visitor_fn empty_layer;
visitor_fn other_layer;

#define DIE_SYS(MSG) do { perror(MSG); exit(1); } while (0)
#define DIE(MSG) do { fputs(MSG, stderr); exit(1); } while (0)

static size_t dirs_index = 0, fds_index = 0;
static charbuf_t path_buf = { .buf = NULL, .cap = 0, .idx = 0 };
static DIR *dirs[DIR_BUF_SIZE] = { 0 };
static int fds[FDS_BUF_SIZE] = { 0 };
void cleanup(void) {
    if (path_buf.buf) buf_destory(&path_buf);
    DIR *dir;
    while ((dir = pop_dir())) if (closedir(dir) < 0) perror("[ERROR]: Failed to close directory");
    assert(!dirs_index);
    int fd;
    while ((fd = pop_fd()) >= 0) if (close(fd) < 0) perror("[ERROR]: Failed to close directory");
    assert(!fds_index);
}

int main(void) {
    atexit(cleanup);
    buf_init(&path_buf, PATH_MAX);
    if (!getcwd(path_buf.buf, path_buf.cap)) DIE_SYS("[ERROR]: Failed to get current working directory");
    path_buf.idx = (char *)memchr(path_buf.buf, 0, path_buf.cap) - path_buf.buf + 1;
    // strcpy(path_buf.buf, "/usr/");
    // path_buf.idx += strlen("/usr/");
    DIR *dir = opendir(path_buf.buf);
    if (!dir) DIE_SYS("[ERROR]: Failed to open root directory");
    push_dir(dir);
    process_directories(dir, &path_buf, first_layer);
    rewinddir(dir);
    process_directories(dir, &path_buf, other_layer);
    return 0;
}

char *convert_to_readable(time_t time) {
    // Day -> XXX
    // Mon -> XXX
    // Year -> XXXX
    char *result = malloc(sizeof(*result) * (3 + 3 + 4 + 1 * 2 + 1));
    if (!result) return NULL;
    struct tm time_info;
    if (!gmtime_r(&time, &time_info)) return NULL;
    sprintf(result, "%s %s %4u", days[time_info.tm_wday], months[time_info.tm_mon], time_info.tm_year + 1900);
    return result;
}

void process_directories(DIR *dir, charbuf_t *path_buf, visitor_fn *visitor) {
    errno = 0;
    for (struct dirent *entry = readdir(dir); entry; entry = readdir(dir)) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        if (!buf_push_path(path_buf, entry->d_name)) continue;
        errno = 0;
        int fd = open(path_buf->buf, O_RDONLY | O_NOFOLLOW, 0666);
        if (fd < 0) {
            if (errno == ELOOP) {
                visitor(NULL, path_buf->buf);
                goto end;
            }
            if (errno == EACCES) goto end;
            printf("%s\n", path_buf->buf);
            DIE_SYS("[ERROR]: Failed to open file");
        }
        if (push_fd(fd)) {
            struct stat einfo;
            if (fstat(fd, &einfo) < 0) DIE_SYS("[ERROR]: Failed to read data");
            switch (einfo.st_mode & S_IFMT) {
            case S_IFDIR:
                DIR *nextdir = opendir(path_buf->buf);
                if (nextdir) {
                    if (push_dir(nextdir))
                        process_directories(nextdir, path_buf, (visitor_fn *)visitor(&einfo, path_buf->buf));
                    if (closedir(pop_dir()) < 0) DIE_SYS("[ERROR]: Failed to close dir");
                }
                break;
            default:
                visitor(&einfo, path_buf->buf);
            }
            if (close(pop_fd()) < 0) DIE_SYS("[ERROR]: Failed to close file");
        }
end:
        if (!buf_pop_path(path_buf)) DIE("[ERROR]: Failed to retrieve path\n");
        errno = 0;
    }
    if (errno) DIE_SYS("[ERROR]: Failed to read dir");
}

void *(*first_layer(struct stat *einfo, const char *name))(struct stat *, const char *) {
    /* symlink */
    if (!einfo) {
        printf("Symlink: `%s`", strrchr(name, '/') + 1);
        static char symlink_buf[PATH_MAX + 1];
        memset(symlink_buf, 0, PATH_MAX + 1);
        if (readlink(name, symlink_buf, PATH_MAX) < 0) DIE_SYS("[ERROR]: Failed to read symlink");
        printf(" -> `%s`\n", symlink_buf);
        return (void *)empty_layer;
    }
    switch (einfo->st_mode & S_IFMT) {
    case S_IFDIR: return (void *)empty_layer;
    default:
    }
    return (void *)empty_layer;
}
void *(*empty_layer(struct stat *einfo, const char *name))(struct stat *, const char *) {
    (void)einfo;
    (void)name;
    return (void *)empty_layer;
}

void *(*other_layer(struct stat *einfo, const char *name))(struct stat *, const char *) {
    if (!einfo) return (void *)other_layer; /* symlink */
    switch (einfo->st_mode & S_IFMT) {
    case S_IFREG:
        if ((einfo->st_mode & 0550) == 0550) {
            printf("R/X file: `%s`", strrchr(name, '/') + 1);
            char *timestamp = convert_to_readable(einfo->st_mtime);
            if (timestamp) {
                printf(" last modified at %s\n", timestamp);
                free(timestamp);
            }
        }
        break;
    default:
    }
    return (void *)other_layer;
}

bool push_dir(DIR *dir) {
    if (dirs_index == DIR_BUF_SIZE) return false;
    dirs[dirs_index++] = dir;
    return true;
}

DIR *pop_dir(void) {
    if (!dirs_index) return NULL;
    return dirs[--dirs_index];
}

bool push_fd(int fd) {
    if (fds_index == FDS_BUF_SIZE) return false;
    fds[fds_index++] = fd;
    return true;
}
int pop_fd(void) {
    if (!fds_index) return -1;
    return fds[--fds_index];
}

void buf_init(charbuf_t *buf, size_t cap) {
    buf->buf = malloc(sizeof(*buf->buf) * cap);
    if (!buf->buf) DIE("Failed to malloc\n");
    memset(buf->buf, 0, buf->cap);
    buf->cap = cap;
    buf->idx = 1;
}

void buf_destory(charbuf_t *buf) {
    if (buf->buf) free(buf->buf);
}

bool buf_push_path(charbuf_t *buf, const char *path) {
    if (strlen(path) + 1 < buf->cap - buf->idx) {
        strcat(buf->buf, "/");
        strcat(buf->buf, path);
        buf->idx += strlen(path) + 1;
        return true;
    }
    return false;
}

char *buf_pop_path(charbuf_t *buf) {
    char *at = strrchr(buf->buf, '/');
    if (at == buf->buf) return NULL;
    *at = 0;
    buf->idx = at - buf->buf + 1;
    return at + 1;
}

