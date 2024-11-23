#include "token.h"

#include <assert.h>

size_t token_operator_precedence(enum TokenOperator op) {
    assert(op != TOKEN_OPERATOR_SIZE);
    switch (op) {
    case TOKEN_OPERATOR_PIPE:
    case TOKEN_OPERATOR_ARBR:
    case TOKEN_OPERATOR_DARBR:
    case TOKEN_OPERATOR_ALBR:
        return 3;
    case TOKEN_OPERATOR_AND:
    case TOKEN_OPERATOR_OR:
        return 2;
    case TOKEN_OPERATOR_SEMICOLON:
    case TOKEN_OPERATOR_AMP:
        return 1;
    default:
        assert(0 && "Invalid token operator passed");
    }
    assert(0 && "Unreachable token_operator_precedence");
}
