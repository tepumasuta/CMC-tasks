/* Если в течение двух секунд придёт два SIGINT, выйти из программы
*/

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void interrput_handler2(int) {
    exit(0);
}

void interrput_handler(int) {
    signal(SIGINT, interrput_handler2);
    puts("Recieved ^C (sigint)");
    alarm(2);
}

void alarm_handler(int) {
    signal(SIGALRM, alarm_handler); // Обработчик сбрасывается после прихода его вызова при поступлении сигнала
    signal(SIGINT, interrput_handler);
}

int main(void) {
    signal(SIGINT, interrput_handler);
    signal(SIGALRM, alarm_handler);
    while (1) pause();
    return 0;
}
