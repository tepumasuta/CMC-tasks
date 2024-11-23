#ifndef LEXER_H
#define LEXER_H 1

#include "token.h"
#include "arena.h"

extern const string_view_t spacing_symbols;

enum LexerError {
    LEXER_ERROR_NONE = 0,
    LEXER_ERROR_NO_MATCHING_QUOT,
    LEXER_ERROR_FAILED_TO_ALLOC,
};

enum LexerError lexer_lex(string_view_t command, struct ArenaStatic **symtable,
                          token_view_t *lexed_tokens, struct ArenaDynamic *token_allocator);

#endif
