#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// #define BUF_SIZE 1 (for spaces testcase)

#define BUF_SIZE (2048 * 4)

void usage(void) {
    puts("Usage: amore [-s] [-num] [+linenum] <filename>\n"
         "\n"
         "Options:\n"
         "    -s – предписывает заменять последовательность пустых строк одной пустой строкой.\n"
         "    -num - целое число задает количество выводимых за один раз строк.\n"
         "    Длина одной порции по умолчанию, т.е. в том случае, если эта опция не задана, 10 строк.\n"
         "    +linenum - начать вывод со строки с номером linenum (целое число >=1)\n"
         "    Нажатие <q> - прервать работу (дальше не выводить файл).");
}

enum options {
    EMPTY_LINES = 1,
    PER_LINE = 2,
    START_FROM = 3,
    INVALID = 4,
};

bool try_parse_full_int(const char *string, size_t *into) {
    if (!string) return false;
    size_t result = 0;
    while (*string) {
        char c = *string;
        if (c <= '9' && c >= '0') result = result * 10 + c - '0';
        else return false;
        string++;
    }
    if (into) *into = result;
    return true;
}

enum options try_parse_arg(const char *arg, size_t *data) {
    if (!arg) return INVALID;
    if (strcmp(arg, "-s") == 0) return EMPTY_LINES;
    if (strlen(arg) < 1) return INVALID;
    if (!try_parse_full_int(arg + 1, data)) return INVALID;
    switch (arg[0]) {
    case '+': return START_FROM;
    case '-': return PER_LINE;
    }
    return INVALID;
}

void skip_line(FILE *file) {
    int c;
    while ((c = fgetc(file)) != '\n' && !feof(file));
}

void print_line(FILE *file, bool compress_lines) {
    static char line_part_buffer[BUF_SIZE] = { 0 };
    static bool prev_empty = false;
    void *success = NULL;
    do {
        size_t repeat = 0;
        while ((success = fgets(line_part_buffer, BUF_SIZE, file))) {
            if (!compress_lines || !prev_empty || line_part_buffer[0] != '\n') printf("%s", line_part_buffer);
            if (strrchr(line_part_buffer, '\n')) break;
            repeat++;
        }
        if (line_part_buffer[0] == '\n' && prev_empty) continue;
        prev_empty = line_part_buffer[0] == '\n' || repeat > 1;
    } while (compress_lines && prev_empty && success);
}

static struct termios oldt, newt;

void cleanup(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 5) {
        usage();
        exit(1);
    }

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // изменение
    atexit(cleanup);
    
    size_t lines_per_read = 10;
    size_t starting_line = 0;
    bool compress_lines = false;
    
    enum options last_arg = 0;
    for (int i = 1; i < argc; i++) {
        size_t value;
        enum options option = try_parse_arg(argv[i], &value);
        if (option < last_arg) {
            fputs("[ERROR]: Wrong flag order\n", stderr);
            usage();
            exit(1);
        }
        switch (option) {
        case EMPTY_LINES:
            compress_lines = true;
            break;
        case PER_LINE:
            lines_per_read = value;
            /* Не было прописано в ТЗ, но не имеет смысла */
            if (!value) {
                fputs("[ERROR]: `num` should be >= 1\n", stderr);
                usage();
                exit(1);
            }
            break;
        case START_FROM:
            if (!value) {
                fputs("[ERROR]: `linenum` should be >= 1\n", stderr);
                usage();
                exit(1);
            }
            starting_line = value;
            break;
        case INVALID:
            if (i != argc - 1) {
                fprintf(stderr, "[ERROR]: Unknown argument: `%s`\n", argv[i]);
                usage();
                exit(1);
            }
            break;
        }
        last_arg = option;
    }

    const char *filename = argv[argc - 1];
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "[ERROR]: Failed to open file `%s`\n", filename);
        perror("");
        exit(1);
    }

    for (size_t i = 0; i < starting_line; i++)
        skip_line(file);

    char c = ' ';
    while (c != 'q' && !feof(file)) {
        if (c == ' ')
            for (size_t i = 0; i < lines_per_read; i++)
                print_line(file, compress_lines);

        if (feof(file)) break;

        c = getchar();
    }
    if (fclose(file)) {
        fprintf(stderr, "[ERROR]: Failed to close file `%s`\n", filename);
        perror("");
        exit(1);
    }
    return 0;
}
