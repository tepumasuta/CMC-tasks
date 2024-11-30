#include "repl.h"
#include "shell.h"

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

char *repl_read_string(struct ArenaDynamic *allocator, enum ReplError *error) {
    assert(allocator);
    assert(error);
    assert(allocator->cap >= 2);
    arena_dynamic_reset(allocator);
    char *last_write;
    errno = 0;
    while (last_write = fgets(allocator->memory + allocator->at, allocator->cap, stdin)) {
        size_t more_length = strlen(last_write);
        allocator->at += more_length;
        if (((char *)allocator->memory)[allocator->at - 1] == '\n') break;
        if (!arena_dynamic_try_reserve(allocator, allocator->cap * 2)) {
            *error = REPL_ERROR_FAILED_TO_ALLOC;
            return NULL;
        }
        errno = 0;
    }
    if (!last_write && errno != 0) { assert(0 && "TODO: Handle error"); }
    return allocator->at ? allocator->memory : NULL;
}

void repl_print_input_promt(struct Shell *shell, const struct REPLSettings settings) {
    if (shell->last_exit_code) {
        if (settings.colorized) printf("\033[91m");
        printf("[%d]", shell->last_exit_code);
        if (settings.colorized) printf("\033[0m");
    }
    if (settings.colorized) printf("\033[92m");
    printf("%s", shell->login);
    if (settings.colorized) printf("\033[0m");
    printf("@");
    if (settings.colorized) printf("\033[94m");
    printf("%s", shell->cwd);
    if (settings.colorized) printf("\033[0m");
    printf(">");
    fflush(stdout);
}

void repl_print_fatal(struct REPLSettings settings, const char *message) {
    if (settings.colorized) fprintf(stderr, "\033[91m");
    fprintf(stderr, "[FATAL]: %s\n", message);
    if (settings.colorized) fprintf(stderr, "\033[0m");
}

void repl_print_error(struct REPLSettings settings, const char *message) {
    if (settings.colorized) fprintf(stderr, "\033[91m");
    fprintf(stderr, "[ERROR]: %s\n", message);
    if (settings.colorized) fprintf(stderr, "\033[0m");
}

void repl_printf_error(struct REPLSettings settings, const char *message, ...) {
    if (settings.colorized) fprintf(stderr, "\033[91m");
    fputs("[ERROR]: ", stderr);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    if (settings.colorized) fprintf(stderr, "\033[0m");
}

static void print(const char *string) {
    fputs(string, stdout);
}

struct HSL {
    double h, s, l;
};

struct RGB {
    int r, g, b;
};

#define MAX(A,B) (((A)>(B))?(A):(B))
#define MIN(A,B) (((A)<(B))?(A):(B))

double hue2rgb(double p, double q, double t) {
    if(t < 0.) t += 1.;
    if(t > 1.) t -= 1.;
    if(t < 1./6.) return p + (q - p) * 6. * t;
    if(t < 1./2.) return q;
    if(t < 2./3.) return p + (q - p) * (2./3. - t) * 6.;
    return p;
}


struct RGB hslToRgb(double h, double s, double l) {
    double r, g, b;

    if (s == 0.) {
        r = g = b = l; // achromatic
    } else {
        double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        double p = 2. * l - q;
        r = hue2rgb(p, q, h + 1./3.);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1./3.);
    }

    return (struct RGB){ .r=(r * 255.), .g=(g * 255.), .b=(b * 255.) };
}


const char *message = "TODO: Write a message";

void repl_colorful_welcome(void) {
    // "\033[38;2;146;255;12mTEXT\033[0m"
    int j = 0;
    for (int i = 0; i < strlen(message); i++) {
        print("\033[38;2");
        struct RGB rgb = hslToRgb((sin(0.15 * j + 1.93) + 1.0) / 2.0, 0.85, 0.75);
        printf(";%d", rgb.r);
        printf(";%d", rgb.g);
        printf(";%d", rgb.b);
        printf("m%c", message[i]);
        if (message[i] != ' ' && message[i] != '\t' && message[i] != '\n') j++;
    }
    printf("\033[0m\n");
}
