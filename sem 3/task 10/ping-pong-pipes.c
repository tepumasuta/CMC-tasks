#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define S1(x) #x
#define S2(x) S1(x)

#define DIE_USAGE(MSG) do { fprintf(stderr, "[FATAL:"S2(__LINE__)"]: " MSG "\n"); usage(); exit(1); } while (0)
#define DIE(MSG) do { fprintf(stderr, "[FATAL:"S2(__LINE__)"]: " MSG "\n"); exit(1); } while (0)
#define DIE_SYS(MSG) do { perror("[FATAL:"S2(__LINE__)"]: " MSG); exit(1); } while(0)
#define ERR_SYS(MSG) do { perror("[ERROR:"S2(__LINE__)"]: " MSG); } while(0)
#define WARN(MSG) do { fprintf(stderr, "[WARNING:"S2(__LINE__)"]: " MSG "\n"); } while(0)

void usage(void);

static int step = 1;

int main(int argc, char *argv[]) {
    if (argc < 4) DIE_USAGE("Too few arguments");
    const int processes_count = atoi(argv[1]);
    const int from = atoi(argv[2]);
    const int to = atoi(argv[3]);
    if (processes_count < 2) DIE_USAGE("Number of processes has to be at least 2");
    if (from == to) {
        WARN("Trying to ping-pong with one value, doesn't make sense, no processes created");
        printf("Process %d, number %d\n", getpid(), from);
        return 0;
    }

    if (from > to) step = -1;


    const pid_t pid = getpid();
    int (*fdss)[2] = malloc(sizeof(*fdss) * processes_count);
    if (!fdss) DIE_SYS("Failed to malloc");
    for (int i = 0; i < processes_count; i++) {
        if (pipe(fdss[i]) < 0) {
            for (int j = 0; j < i; j++) {
                if (close(fdss[i][0]) < 0) ERR_SYS("Failed to close");
                if (close(fdss[i][1]) < 0) ERR_SYS("Failed to close");
            }
            DIE_SYS("Failed to pipe");
        }
    }

    int fds[2] = { -1, -1 };

    int process_number = 1;
    for (int i = 1; i < processes_count; i++) { // N - 1 forks
        process_number++;
        const pid_t p = fork();
        if (p < 0) {
            for (int j = 0; j < processes_count; j++) {
                if (close(fdss[j][0]) < 0) ERR_SYS("Failed to close");
                if (close(fdss[j][1]) < 0) ERR_SYS("Failed to close");
            }
            DIE_SYS("Failed to fork");
        } else if (!p) {
            for (int j = 0; j < i - 1; j++) {
                if (close(fdss[j][0]) < 0) ERR_SYS("Failed to close");
                if (close(fdss[j][1]) < 0) ERR_SYS("Failed to close");
            }
            for (int j = i + 1; j < processes_count; j++) {
                if (close(fdss[j][0]) < 0) ERR_SYS("Failed to close");
                if (close(fdss[j][1]) < 0) ERR_SYS("Failed to close");
            }
            if (close(fdss[i - 1][1]) < 0) ERR_SYS("Failed to close");
            if (close(fdss[i][0]) < 0) ERR_SYS("Failed to close");
            fds[0] = fdss[i - 1][0];
            fds[1] = fdss[i][1];
            break;
        }
    }
    if (pid == getpid()) {
        process_number = 1;
        for (int i = 1; i < processes_count - 1; i++) {
            if (close(fdss[i][0]) < 0) ERR_SYS("Failed to close");
            if (close(fdss[i][1]) < 0) ERR_SYS("Failed to close");
        }
        if (close(fdss[0][0]) < 0) ERR_SYS("Failed to close");
        if (close(fdss[processes_count - 1][1]) < 0) ERR_SYS("Failed to close");
        fds[0] = fdss[processes_count - 1][0];
        fds[1] = fdss[0][1];
    }
    free(fdss);

    int num = from;
    ssize_t read_result;
    if (pid == getpid()) {
        printf("Process PID %d (%d), number: %d\n", getpid(), process_number, num);
        num += step;
        ssize_t write_result;
        if ((write_result = write(fds[1], &num, sizeof(num))) < (ssize_t)sizeof(num)) {
            if (close(fds[0]) < 0) ERR_SYS("Failed to close");
            if (close(fds[1]) < 0) ERR_SYS("Failed to close");
            if (write_result < 0) DIE_SYS("Failed to write");
            DIE("Failed to write");
        }
    }
    while ((read_result = read(fds[0], &num, sizeof(num))) == sizeof(num)) {
        printf("Process PID %d (%d), number: %d\n", getpid(), process_number, num);
        if (num == to) goto end;
        num += step;
        int write_result;
        if ((write_result = write(fds[1], &num, sizeof(num))) < (ssize_t)sizeof(num)) {
            if (close(fds[0]) < 0) ERR_SYS("Failed to close");
            if (close(fds[1]) < 0) ERR_SYS("Failed to close");
            if (write_result < 0) DIE_SYS("Failed to write");
            DIE("Failed to write");
        }
    }
    if (read_result != 0) {
        if (close(fds[0]) < 0) ERR_SYS("Failed to close");
        if (close(fds[1]) < 0) ERR_SYS("Failed to close");
        if (read_result < 0) DIE_SYS("Failed to read");
        DIE("Failed to read");
    }

end:
    if (close(fds[0]) < 0) ERR_SYS("Failed to close");
    if (close(fds[1]) < 0) ERR_SYS("Failed to close");

    return 0;
}

void usage(void) {
    puts("Usage: ./ping-pong <processes_count> <from> <to>");
}
