#include <signal.h>
#include <unistd.h>
#include <stdio.h>

// Да сбрасывается, программа падает

void sigtrap_handler(int) {
    printf("SIGTRAP handled!\n");
}

int main(void) {
    signal(SIGTRAP, sigtrap_handler);
    raise(SIGTRAP);
    raise(SIGTRAP);
    return 0;
}
