/* Определить, возникает ли процесс зомби, если сигнал SIGCHLD родительским процессов:
   - Перехватывается
   - Не перехватывается
   - Игнорируется
*/

// Как я чекал:
// Запускал, нажимал ^Z, запускал ps, если процесса 2, то один из них сын зомби, после чего fg и ^C

#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#define CATCH 0 // Да, возникает
#define IGNORE 1 // Нет, не возникает
#define PASS 2 // Да, возникает

#define CHOOSE PASS

void handler(int) {
    printf("SIGCHLD caught!\n");
    fflush(stdout);
}

int main(void) {
#if (CHOOSE == CATCH)
    signal(SIGCHLD, handler);
#elif (CHOOSE == IGNORE)
    signal(SIGCHLD, SIG_IGN);
#elif (CHOOSE == PASS)
#endif

    if (!fork()) {
        printf("%u\n", getpid());
        return 0;
    }
    printf("%u\n", getpid());

    while(1);
    return 0;
}
