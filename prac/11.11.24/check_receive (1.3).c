#define _POSIX_C_SOURCE 1

// Процесс умирает, так что да, получает

#include <signal.h>
#include <unistd.h>

int main(void) {
    kill(0, SIGABRT);
    return 0;
}
