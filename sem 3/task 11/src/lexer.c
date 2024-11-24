#include "lexer.h"
#include "defer.h"

#include <assert.h>
#include <string.h>

const string_view_t spacing_symbols = SV_FROM_CSTR(" \t");
static const string_view_t disallowed_name_symbols = SV_FROM_CSTR(" \t;()|&<>");

static bool chop_if_starts_with(string_view_t *view, string_view_t prefix) {
    if (sv_starts_with(*view, prefix)) {
        sv_chop_n(view, prefix.length);
        return true;
    }
    return false;
}

static bool try_lex_syntax(string_view_t *command, struct Token *into) {
    assert(command);
    assert(into);
    struct Token token = (struct Token){ .as_syntax = TOKEN_SYNTAX_SIZE };
    
    static_assert(TOKEN_SYNTAX_SIZE == 2);
    if (chop_if_starts_with(command, SV_FROM_CSTR("("))) { token.as_syntax = TOKEN_SYNTAX_OPAREN; }
    else if (chop_if_starts_with(command, SV_FROM_CSTR(")"))) { token.as_syntax = TOKEN_SYNTAX_CPAREN; }

    if (token.as_syntax != TOKEN_SYNTAX_SIZE) {
        token.type = TOKEN_TYPE_SYNTAX;
        *into = token;
        return true;
    }

    return false;
}
static bool try_lex_operation(string_view_t *command, struct Token *into) {
    assert(command);
    assert(into);
    struct Token token = (struct Token){ .as_operator = TOKEN_OPERATOR_SIZE };
    
    static_assert(TOKEN_OPERATOR_SIZE == 8);
    if (chop_if_starts_with(command, SV_FROM_CSTR("&&"))) { token.as_operator = TOKEN_OPERATOR_AND; }
    else if (chop_if_starts_with(command, SV_FROM_CSTR("||"))) { token.as_operator = TOKEN_OPERATOR_OR; }
    else if (chop_if_starts_with(command, SV_FROM_CSTR(">>"))) { token.as_operator = TOKEN_OPERATOR_DARBR; }
    else if (chop_if_starts_with(command, SV_FROM_CSTR("|"))) { token.as_operator = TOKEN_OPERATOR_PIPE; }
    else if (chop_if_starts_with(command, SV_FROM_CSTR(">"))) { token.as_operator = TOKEN_OPERATOR_ARBR; }
    else if (chop_if_starts_with(command, SV_FROM_CSTR("<"))) { token.as_operator = TOKEN_OPERATOR_ALBR; }
    else if (chop_if_starts_with(command, SV_FROM_CSTR("&"))) { token.as_operator = TOKEN_OPERATOR_AMP; }
    else if (chop_if_starts_with(command, SV_FROM_CSTR(";"))) { token.as_operator = TOKEN_OPERATOR_SEMI; }

    if (token.as_operator != TOKEN_OPERATOR_SIZE) {
        token.type = TOKEN_TYPE_OPERATOR;
        *into = token;
        return true;
    }

    return false;
}
static bool try_lex_symbol(string_view_t *command, struct Token *into, enum LexerError *error) {
    assert(command);
    assert(into);
    string_view_t view = *command;

    if (sv_starts_with(view, SV_FROM_CSTR("\""))) {
        sv_chop(&view);
        string_view_t next_quot = sv_find_symbol(view, '"');
        if (!next_quot.start) {
            *error = LEXER_ERROR_NO_MATCHING_QUOT;
            return true;
        }
        view.length = next_quot.start - view.start;
        sv_chop(&next_quot);
        *command = next_quot;
        *into = (struct Token){ .type = TOKEN_TYPE_SYMBOL, .as_symbol = (struct TokenSymbol){ .symbol = view } };
        return true;
    } else {
        assert(!sv_starts_with(view, SV_FROM_CSTR(" ")));
        assert(!sv_starts_with(view, SV_FROM_CSTR("\t")));
        string_view_t left = sv_find_symbol_any(view, disallowed_name_symbols);
        if (!left.start) {
            *into = (struct Token){ .type = TOKEN_TYPE_SYMBOL, .as_symbol = (struct TokenSymbol){ .symbol = view } };
            sv_chop_n(command, view.length);
            return true;
        }
        view.length = left.start - view.start;
        *command = left;
        *into = (struct Token){ .type = TOKEN_TYPE_SYMBOL, .as_symbol = (struct TokenSymbol){ .symbol = view } };
        return true;
    }
}


static bool try_lex_token(string_view_t *command, struct Token *into, enum LexerError *error) {
    assert(command);
    assert(into);
    if (!command->length) return false;
    static_assert(TOKEN_TYPE_SIZE == 3);
    if (try_lex_syntax(command, into)
        || try_lex_operation(command, into)
        || try_lex_symbol(command, into, error))
        return true;
    return false;
}

enum LexerError lexer_lex(string_view_t command, struct ArenaStatic **symtable,
                          token_view_t *lexed_tokens, struct ArenaDynamic *token_allocator) {
    assert(symtable);
    assert(token_allocator);
    assert(lexed_tokens);
    size_t tokens_at = token_allocator->at;
    enum LexerError error = LEXER_ERROR_NONE;
    struct Token current;
    struct ArenaStatic *symtable_local = arena_static_try_create(command.length * 2);
    if (!symtable_local) return LEXER_ERROR_FAILED_TO_ALLOC;
    sv_trim_any(&command, spacing_symbols);
    size_t total_tokens = 0;
    while (try_lex_token(&command, &current, &error)) {
        if (error != LEXER_ERROR_NONE) {
            assert(error != LEXER_ERROR_FAILED_TO_ALLOC);
            *symtable = symtable_local;
            *lexed_tokens = (token_view_t){ .start = (void *)((char *)token_allocator->memory + tokens_at), .length = total_tokens };
            return error;
        }
        total_tokens++;
        if (current.type == TOKEN_TYPE_SYMBOL) {
            char *symtable_local_address = arena_static_alloc(symtable_local, current.as_symbol.symbol.length + 1);
            memcpy(symtable_local_address,
                   current.as_symbol.symbol.start,
                   current.as_symbol.symbol.length);
            symtable_local_address[current.as_symbol.symbol.length] = '\0';
            current.as_symbol.symbol.start = symtable_local_address;
        }
        ArenaOffset offset;
        if (!arena_dynamic_try_alloc(token_allocator, sizeof(current), &offset)) {
            arena_static_destroy(&symtable_local);
            return LEXER_ERROR_FAILED_TO_ALLOC;
        }
        *(struct Token *)arena_dynamic_get_memory(token_allocator, offset) = current;
        sv_trim_any(&command, spacing_symbols);
    }
    assert(command.length == 0);
    *symtable = symtable_local;
    *lexed_tokens = (token_view_t){ .start = (void *)((char *)token_allocator->memory + tokens_at), .length = total_tokens };
    return LEXER_ERROR_NONE;
}
