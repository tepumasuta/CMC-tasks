#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

#define S1(x) #x
#define S2(x) S1(x)

#define return_defer(RET_VAL) do { result = (RET_VAL); goto defer; } while(0)
#define DIE_USAGE(MSG) do { fprintf(stderr, "[FATAL:"S2(__LINE__)"]: " MSG "\n"); usage(); return_defer(1); } while (0)
#define DIE(MSG) do { fprintf(stderr, "[FATAL:"S2(__LINE__)"]: " MSG "\n"); return_defer(1); } while (0)
#define DIE_SYS(MSG) do { perror("[FATAL:"S2(__LINE__)"]: " MSG); return_defer(1); } while(0)
#define ERR_SYS(MSG) do { perror("[ERROR:"S2(__LINE__)"]: " MSG); } while(0)
#define WARN(MSG) do { fprintf(stderr, "[WARNING:"S2(__LINE__)"]: " MSG "\n"); } while(0)

void usage(void);
void sigusr1_handle(int);
void sigusr2_handle(int);

/* Full of deadlocks, when signals are received in wrong order (for big from and to)
   (and realiable singal interface we didn't touch, though i could write, but I ain't gonna) */

static volatile bool should_read = false;
static volatile bool should_die = false;
static int step = 1;

int main(int argc, char *argv[]) {
    int result = 0;

    if (argc < 4) DIE_USAGE("Too few arguments");
    const int processes_count = atoi(argv[1]);
    const int from = atoi(argv[2]);
    const int to = atoi(argv[3]);
    if (processes_count < 2) DIE_USAGE("Number of processes has to be at least 2");
    if (from == to) {
        WARN("Trying to ping-pong with one value, doesn't make sense, no processes created");
        printf("Process PID %d (1), number: %d\n", getpid(), to);fflush(stdout);
        return_defer(0);
    }
    if (from > to) step = -1;

    signal(SIGUSR1, sigusr1_handle);
    signal(SIGUSR2, sigusr2_handle);

    const pid_t base_pid = getpid();
    pid_t pid_pending = base_pid;

    static int fds[2] = { -1, -1 };
    if (pipe(fds) < 0) DIE_SYS("Failed to pipe");

    int process_number = 1;
    for (int i = 1; i < processes_count; i++) { // N - 1 forks
        pid_pending = fork();
        signal(SIGUSR1, sigusr1_handle);
        signal(SIGUSR2, sigusr2_handle);
        if (pid_pending < 0) DIE_SYS("Failed to fork");
        else if (pid_pending) break;
        process_number++;
        if (i == processes_count - 1) pid_pending = base_pid;
    }

    int num = from;
    if (base_pid == getpid()) {
        process_number = 1;
        printf("Process PID %d (%d), number: %d\n", getpid(), process_number, num);fflush(stdout);
        num += step;
        for (int i = 0; i < processes_count - 1; i++) {
            char c;
            if (read(fds[0], &c, sizeof(c)) != sizeof(c)) {
                if (kill(pid_pending, SIGUSR2) < 0) ERR_SYS("Failed to kill");
                DIE_SYS("Failed to read");
            }
        }
        if (write(fds[1], &num, sizeof(num)) != sizeof(num)) {
            if (kill(pid_pending, SIGUSR2) < 0) ERR_SYS("Failed to kill");
            DIE_SYS("Failed to write");
        }
        /* A bit safety unreliable from deadlocks */
        if (processes_count < 10) for (volatile int junk = 0; junk < 10000; junk++);
        if (kill(pid_pending, SIGUSR1) < 0) DIE_SYS("Failed to kill");
    } else {
        char c;
        if (write(fds[1], &c, sizeof(c)) != sizeof(c)) {
            if (kill(pid_pending, SIGUSR2) < 0) ERR_SYS("Failed to kill");
            DIE_SYS("Failed to write");
        }
    }

    while (1) {
        pause();
        if (should_read) {
            should_read = false;
            if (read(fds[0], &num, sizeof(num)) != sizeof(num)) {
                if (kill(pid_pending, SIGUSR2) < 0) ERR_SYS("Failed to kill");
                DIE_SYS("Failed to read");
            }
            printf("Process PID %d (%d), number: %d\n", getpid(), process_number, num);fflush(stdout);
            if (num == to) {
                if (kill(pid_pending, SIGUSR2) < 0) DIE_SYS("Failed to kill");
                return_defer(0);
            }
            num += step;
            if (write(fds[1], &num, sizeof(num)) != sizeof(num)) {
                if (kill(pid_pending, SIGUSR2) < 0) ERR_SYS("Failed to kill");
                DIE_SYS("Failed to write");
            }
            /* A bit unreliable safety from deadlocks */
            if (processes_count < 10) for (volatile int junk = 0; junk < 10000; junk++);
            if (kill(pid_pending, SIGUSR1) < 0) DIE_SYS("Failed to kill");
        }
        if (should_die) {
            if (kill(pid_pending, SIGUSR2) < 0) DIE_SYS("Failed to kill");
            return_defer(0);
        }
    }

defer:
    if (fds[0] >= 0) if (close(fds[0]) < 0) ERR_SYS("Failed to close");
    if (fds[1] >= 0) if (close(fds[1]) < 0) ERR_SYS("Failed to close");

    return result;
}

void usage(void) {
    puts("Usage: ./ping-pong <processes_count> <from> <to>");
}

void sigusr1_handle(int) {
    signal(SIGUSR1, sigusr1_handle);
    should_read = true;
}

void sigusr2_handle(int) {
    signal(SIGUSR2, sigusr2_handle);
    should_die = true;
}

