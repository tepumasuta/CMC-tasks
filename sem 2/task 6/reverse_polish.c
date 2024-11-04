#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "stack.h"

#define DIE_WITH_MESSAGE(...) do { fprintf(stderr, __VA_ARGS__); return EXIT_FAILURE; } while (0);

enum associativity {
    RIGHT_ASSOCIATIVE,
    LEFT_ASSOCIATIVE,
    ASSOCIATIVITY_SIZE,
};

typedef struct {
    char sym;
    bool was_digit;
} op_context_t;

static unsigned get_precedence(char sym) {
    switch (sym) {
    case '-':
    case '+':
        return 1;
    case '/':
    case '*':
    case '%':
        return 2;
    case 'm':
    case 'p':
        return 3;
    default: return 0;
    }
}

static enum associativity get_associativity(char sym) {
    switch (sym) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
        return LEFT_ASSOCIATIVE;
    case 'm':
    case 'p':
        return RIGHT_ASSOCIATIVE;
    default:
        return ASSOCIATIVITY_SIZE;
    }
}

static char resolve_operation(op_context_t context) {
    switch (context.sym) {
    case '+': return context.was_digit ? '+' : 'p';
    case '-': return context.was_digit ? '-' : 'm';
    }
    return context.sym;
}

static void print_op(char op) {
    switch (op) {
    case 'm': printf("- "); break;
    case 'p': printf("+ "); break;
    default:
        printf("%c ", op);
    }
}

static inline bool is_unary(char op) {
    return op == 'p' || op == 'm';
}

static inline bool is_operator(char op) {
    return op == '+' || op == '-' || op == '*' || op == '/';
}

int main(void) {
    assert(stack_cap() > 1 && "[ASSERT_ERROR] Stack size too small, unable to operate");
    stack_reset();
    stack_push('(');

    int char_buf, char_prev = '\0';
    unsigned left_brackets = 0;
    bool was_digit = false, was_nonunary = true, read_anything = false, was_nonbracket = false;
    while ((char_buf = getchar()) != EOF && char_buf != '=' && (left_brackets || char_buf != '\n')) {
        // Skip whitespace
        if (isspace(char_buf)) continue;
        if (isdigit(char_buf)) {
            if (char_prev == ')') DIE_WITH_MESSAGE("[ERROR]: Incorrect exprission, implicit multiplication on parenthesis is not supported\n");
            was_nonbracket = true;
            read_anything = true;
            if (was_digit) DIE_WITH_MESSAGE("[ERROR]: Received two consequent numbers without operation\n");
            do {
                putc(char_buf, stdout);
                char_buf = getchar();
            } while (isdigit(char_buf));
            ungetc(char_buf, stdin);
            was_digit = true;
            was_nonunary = false;
            putc(' ', stdout);
        } else switch (char_buf) {
        case '+':
        case '-':
        case '*':
        case '/':
            char op_sym = resolve_operation((op_context_t){ .sym = char_buf, .was_digit = was_digit });
            enum associativity as = get_associativity(op_sym);
            assert(as != ASSOCIATIVITY_SIZE || (fprintf(stderr, "[ASSERT_ERROR]: Incorrectly parsed operation: `%c`\n", op_sym) && 0));

            bool left_associative = as == LEFT_ASSOCIATIVE;
            unsigned precedence = get_precedence(op_sym);
            assert(precedence || (fprintf(stderr, "[ASSERT_ERROR]: Incorrectly calculated precedence for `%c` (should be unreachable)\n", op_sym) && 0));
            if (was_nonunary && !is_unary(op_sym)) DIE_WITH_MESSAGE("[ERROR]: %s\n", read_anything ? "Two consequent binary operations" : "Starting with binary operator");

            char old_op;
            while ((old_op = stack_pop()) != '(') {
                unsigned old_precedence = get_precedence(old_op);
                if (!(old_precedence > precedence || (left_associative && old_precedence == precedence)))
                    break;
                print_op(old_op);
            }
            stack_push(old_op);
            if (!stack_try_push(op_sym)) DIE_WITH_MESSAGE("[ERROR]: Too long expression, couldn't parse after `%c`\n", char_buf);
            
            if (!(was_nonunary = !is_unary(op_sym)))
                printf("0 ");

            was_digit = false;
            was_nonbracket = true;
            read_anything = true;
            break;
        case '(':
            if (was_digit) DIE_WITH_MESSAGE("[ERROR]: Incorrect exprission, implicit multiplication on parenthesis is not supported\n");
            left_brackets++;
            if (!stack_try_push('(')) DIE_WITH_MESSAGE("[ERROR]: Too dense expression, couldn't parse after `(`\n");
            break;
        case ')':
            if (was_nonbracket && char_prev == '(') DIE_WITH_MESSAGE("[ERROR]: Recieved empty braces\n");
            if (is_operator(char_prev)) DIE_WITH_MESSAGE("[ERROR]: Recieved operator before closing brace\n");
            if (!left_brackets) DIE_WITH_MESSAGE("[ERROR]: Incorrect expression, recieved `)` but no matching `(`\n");

            char op;
            while ((op = stack_pop()) != '(')
                print_op(op);
            left_brackets--;
            break;
        default:
            DIE_WITH_MESSAGE("[ERROR]: Unknown start of symbol: `%c`\n", char_buf);
        };
        char_prev = char_buf;
    }


    if (left_brackets) DIE_WITH_MESSAGE("[ERROR]: Unclosed open brackets (%d)\n", left_brackets)
    else if (!was_digit && read_anything) DIE_WITH_MESSAGE("[ERROR]: Unclosed %s operation\n", (was_nonunary ? "binary" : "unary"));


    if (!read_anything) fprintf(stderr, "[WARNING]: Empty sequence\n");
    while ((char_buf = stack_pop()) != '(')
        print_op(char_buf);

    return EXIT_SUCCESS;
}
