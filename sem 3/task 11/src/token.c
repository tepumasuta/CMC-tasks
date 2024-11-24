#include "token.h"

#include <assert.h>

bool token_is_redirection(enum TokenOperator op) {
    assert(op != TOKEN_OPERATOR_SIZE);
    return op == TOKEN_OPERATOR_ARBR || op == TOKEN_OPERATOR_DARBR || op == TOKEN_OPERATOR_ALBR;
    assert(0 && "Unreachable");
}

void tv_chop(token_view_t *token_view) {
    if (token_view->length) {
        token_view->length--;
        token_view->start++;
    }
}

void tv_chop_n(token_view_t *token_view, size_t symbols_count) {
    if (symbols_count > token_view->length) symbols_count = token_view->length;
    token_view->length -= symbols_count;
    token_view->start += symbols_count;
}
