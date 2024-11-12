/* Посчитать сколько было SIGTSTP между вторым и четвртым SIGINT
   После шестого SIGINT выйти из программы и распечатать количество SIGTSTP
*/

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

volatile unsigned sigints = 0;
volatile unsigned sigtstps = 0;

void sigtstp_handler(int) {
    signal(SIGTSTP, sigtstp_handler);
    sigtstps++;
}

void sigtstp_handler(int) {
    signal(SIGINT, sigtstp_handler);
    sigints++;
    if (sigints == 2) signal(SIGTSTP, sigtstp_handler);
    if (sigints == 4) signal(SIGTSTP, SIG_IGN);
    if (sigints == 6) {
        printf("Total SIGTSTP count: %u\n", sigtstps);
        exit(0);
    }
}

int main(void) {
    signal(SIGINT, sigtstp_handler);
    signal(SIGTSTP, SIG_IGN);
    while (1) pause();
    return 0;
}
