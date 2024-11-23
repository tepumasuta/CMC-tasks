#ifndef TOKEN_H
#define TOKEN_H 1

#include "string_view.h"

enum TokenType {
    TOKEN_TYPE_SYNTAX = 0,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_SIZE,
};

enum TokenSyntax {
    TOKEN_SYNTAX_OPAREN = 0, /* Open parenthesis */
    TOKEN_SYNTAX_CPAREN, /* Close parenthesis */
    TOKEN_SYNTAX_SIZE,
};

enum TokenOperator {
    TOKEN_OPERATOR_PIPE = 0,
    TOKEN_OPERATOR_ARBR, /* Angle right bracket */
    TOKEN_OPERATOR_DARBR, /* Double angle right bracket */
    TOKEN_OPERATOR_ALBR, /* Angle left bracket */
    TOKEN_OPERATOR_AND,
    TOKEN_OPERATOR_OR,
    TOKEN_OPERATOR_SEMI,
    TOKEN_OPERATOR_AMP,
    TOKEN_OPERATOR_SIZE,
};

struct TokenSymbol {
    string_view_t symbol;
};

struct Token {
    enum TokenType type;
    union {
        enum TokenSyntax as_syntax;
        enum TokenOperator as_operator;
        struct TokenSymbol as_symbol;
    };
};

typedef struct {
    struct Token *start;
    size_t length;
} token_view_t;

size_t token_operator_precedence(enum TokenOperator op);

#endif
