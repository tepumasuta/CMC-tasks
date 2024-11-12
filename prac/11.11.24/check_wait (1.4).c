#define _POSIX_C_SOURCE 1

// Да, прерывает

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

void handler(int) {
    puts("Received SIGALRM");
    fflush(stdout);
}

int main(void) {
    signal(SIGALRM, handler);
    if (!fork()) {
        for (volatile size_t i = 0; i < 1000000; i++);
        kill(getppid(), SIGALRM);
        for (volatile size_t i = 0; i < 1000000; i++);
        return 0;
    }
    printf("%d\n", wait(NULL));
    printf("%d\n", wait(NULL));
    return 0;
}
