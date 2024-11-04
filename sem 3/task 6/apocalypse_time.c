#define __USE_POSIX 1

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

static const char *const months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};
static const char *const days[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

/* Has to be free-ed */
char *convert_to_readable(time_t time) {
    // Day -> XX
    // Mon -> XXX
    // Year -> XXXX
    char *result = malloc(sizeof(*result) * (2 + 3 + 4 + 1 * 2 + 1));
    if (!result) return NULL;
    struct tm time_info;
    if (!gmtime_r(&time, &time_info)) return NULL;
    sprintf(result, "%s %s %4u", days[time_info.tm_wday], months[time_info.tm_mon], time_info.tm_year + 1900);
    return result;
}

int main(void) {
    const char *now = convert_to_readable(time(0));
    printf("Today is %s...\nHow am I still alive.....\n", now);
    free((char *)now);
    return 0;
}
