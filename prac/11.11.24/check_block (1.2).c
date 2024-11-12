#define _POSIX_C_SOURCE 1

// Результат: не блокируются, доходят по мере поступления
// Закоментированно -- proof of concept, что было бы, блокируйся они

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>

volatile int n = 0;
// volatile bool add = 0;

void sigtstp_handler(int) {
    signal(SIGTSTP, sigtstp_handler);
    int k = n++;
    printf("Started handling %d\n", k);
    fflush(stdout);
    for (volatile size_t i = 0; i < 1000000000; i++);
    printf("Handled %d\n", k);
    fflush(stdout);
}

// void singint_handler(int) {
//     add = true;
// }

void sigabrt_handler(int) {
    signal(SIGABRT, sigabrt_handler);
    printf("Handled SIGABRT\n");
    fflush(stdout);
}

int main(void) {
    // sigset_t s;
    // sigaddset(&s, SIGTSTP);
    // sigprocmask(SIG_BLOCK, &s, NULL);
    // signal(SIGINT, singint_handler);
    
    void (*sigtstp_default)(int) = signal(SIGTSTP, sigtstp_handler);
    void (*sigabrt_default)(int) = signal(SIGABRT, sigabrt_handler);
    if (!fork()) {
        signal(SIGTSTP, sigtstp_default);
        signal(SIGABRT, sigabrt_default);
        kill(getppid(), SIGTSTP);
        for (volatile size_t i = 0; i < 1000000; i++);
        kill(getppid(), SIGTSTP);
        for (volatile size_t i = 0; i < 1000000; i++);
        kill(getppid(), SIGTSTP);
        for (volatile size_t i = 0; i < 1000000; i++);
        kill(getppid(), SIGABRT);
        for (volatile size_t i = 0; i < 1000000; i++);
        kill(getppid(), SIGTSTP);
        return 0;
    }

    // while(!add);
    // sigprocmask(SIG_UNBLOCK, &s, NULL);

    while(1);

    return 0;
}