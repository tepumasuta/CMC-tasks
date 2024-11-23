#include "string_view.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

string_view_t sv_from_cstr(const char *string) {
    return (string_view_t){
        .start = string,
        .length = strlen(string),
    };
}

char *sv_to_cstr(string_view_t string_view) {
    char *result = malloc(sizeof(*result) * (string_view.length + 1));
    if (!result)
        return NULL;
    memcpy(result, string_view.start, string_view.length);
    result[string_view.length] = '\0';
    return result;
}

void sv_to_cstr_at(string_view_t string_view, char buffer[]) {
    memcpy(buffer, string_view.start, string_view.length);
    buffer[string_view.length] = '\0';
}

void sv_chop(string_view_t *string_view) {
    if (!string_view->length)
        return;
    string_view->start++;
    string_view->length--;
}

void sv_chop_n(string_view_t *string_view, size_t symbols_count) {
    if (string_view->length < symbols_count)
        symbols_count = string_view->length;
    string_view->start += symbols_count;
    string_view->length -= symbols_count;
}

void sv_chop_if(string_view_t *string_view, sv_chop_condition_fn condition) {
    size_t chop_count;
    if (condition(*string_view, &chop_count))
        sv_chop_n(string_view, chop_count);
}

void sv_chop_while(string_view_t *string_view, sv_chop_condition_fn condition) {
    size_t chop_count;
    while (string_view->length && condition(*string_view, &chop_count))
        sv_chop_n(string_view, chop_count);
}

void sv_chop_end(string_view_t *string_view) {
    if (string_view->length)
        string_view->length--;
}

void sv_chop_end_n(string_view_t *string_view, size_t symbols_count) {
    if (string_view->length < symbols_count)
        string_view->length = 0;
    else
        string_view->length -= symbols_count;
}

void sv_chop_end_if(string_view_t *string_view, sv_chop_condition_fn condition) {
    size_t chop_count;
    if (condition(*string_view, &chop_count))
        sv_chop_end_n(string_view, chop_count);
}

void sv_chop_end_while(string_view_t *string_view, sv_chop_condition_fn condition) {
    size_t chop_count;
    while (string_view->length && condition(*string_view, &chop_count))
        sv_chop_end_n(string_view, chop_count);
}

void sv_chop_if_va(string_view_t *string_view, sv_chop_condition_va_fn condition, ...) {
    va_list pargs;
    va_start(pargs, condition);
    size_t chop_count;
    if (condition(*string_view, &chop_count, pargs))
        sv_chop_n(string_view, chop_count);
    va_end(pargs);
}

void sv_chop_while_va(string_view_t *string_view, sv_chop_condition_va_fn condition, ...) {
    va_list pargs;
    va_start(pargs, condition);
    size_t chop_count;
    while (string_view->length && condition(*string_view, &chop_count, pargs))
        sv_chop_n(string_view, chop_count);
    va_end(pargs);
}

void sv_chop_end_if_va(string_view_t *string_view, sv_chop_condition_va_fn condition, ...) {
    va_list pargs;
    va_start(pargs, condition);
    size_t chop_count;
    if (condition(*string_view, &chop_count, pargs))
        sv_chop_end_n(string_view, chop_count);
    va_end(pargs);
}

void sv_chop_end_while_va(string_view_t *string_view, sv_chop_condition_va_fn condition, ...) {
    va_list pargs;
    va_start(pargs, condition);
    size_t chop_count;
    while (string_view->length && condition(*string_view, &chop_count, pargs))
        sv_chop_end_n(string_view, chop_count);
    va_end(pargs);
}

void sv_trim(string_view_t *string_view) {
    while (string_view->length && isspace(*string_view->start))
        sv_chop(string_view);
    while (string_view->length && isspace(string_view->start[string_view->length - 1]))
        sv_chop_end(string_view);
}

void sv_trim_any(string_view_t *string_view, string_view_t symbols) {
    while (string_view->length) {
        for (size_t i = 0; i < symbols.length; i++)
            if (*string_view->start == symbols.start[i]) {
                sv_chop(string_view);
                goto end_first;
            }
        break;
end_first:
    }

    while (string_view->length) {
        for (size_t i = 0; i < symbols.length; i++)
            if (string_view->start[string_view->length - 1] == symbols.start[i]) {
                sv_chop_end(string_view);
                goto end_second;
            }
        break;
end_second:
    }
}

bool sv_starts_with(string_view_t string_view, string_view_t prefix) {
    if (string_view.length < prefix.length)
        return false;

    while (prefix.length--)
        if (*string_view.start++ != *prefix.start++)
            return false;

    return true;
}

bool sv_ends_with(string_view_t string_view, string_view_t suffix) {
    if (string_view.length < suffix.length)
        return false;

    string_view.start += string_view.length - suffix.length;

    while (suffix.length--)
        if (*string_view.start++ != *suffix.start++)
            return false;

    return true;
}

bool sv_contains(string_view_t string_view, string_view_t substring) {
    while (string_view.length >= substring.length) {
        if (sv_starts_with(string_view, substring))
            return true;
        sv_chop(&string_view);
    }
    return false;
}

bool sv_empty(string_view_t string_view) {
    return string_view.length == 0;
}

bool sv_is_subview(string_view_t string_view, string_view_t subview) {
    return string_view.start >= subview.start
        && subview.start + subview.length <= string_view.start + string_view.length;
}

string_view_t sv_find(string_view_t string_view, string_view_t prefix) {
    while (string_view.length >= prefix.length && !sv_starts_with(string_view, prefix))
        sv_chop(&string_view);

    if (string_view.length < prefix.length)
        return (string_view_t){ .start = NULL, .length = 0 };
    
    return string_view;
}

string_view_t sv_find_last(string_view_t string_view, string_view_t prefix) {
    const size_t length = string_view.length;
    while (string_view.length >= prefix.length && !sv_ends_with(string_view, prefix))
        sv_chop_end(&string_view);

    if (string_view.length < prefix.length)
        return (string_view_t){ .start = NULL, .length = 0 };

    return (string_view_t){
        .start = string_view.start + string_view.length - prefix.length,
        .length = length - string_view.length + prefix.length,
    };
}

string_view_t sv_find_symbol(string_view_t string_view, char symbol) {
    while (string_view.length && *string_view.start != symbol)
        sv_chop(&string_view);

    if (!string_view.length)
        return (string_view_t){ .start = NULL, .length = 0 };

    return string_view;
}

string_view_t sv_find_symbol_last(string_view_t string_view, char symbol) {
    const size_t length = string_view.length;

    while (string_view.length && string_view.start[string_view.length - 1] != symbol)
        sv_chop_end(&string_view);
    
    if (!string_view.length)
        return (string_view_t){ .start = NULL, .length = 0 };

    return (string_view_t){
        .start = string_view.start + string_view.length - 1,
        .length = length - string_view.length + 1
    };
}

string_view_t sv_find_symbol_any(string_view_t string_view, string_view_t symbols) {
    while (string_view.length) {
        for (size_t i = 0; i < symbols.length; i++)
            if (*string_view.start == symbols.start[i])
                return string_view;
        sv_chop(&string_view);
    }

    return (string_view_t){ .start = NULL, .length = 0 };
}

string_view_t sv_find_if(string_view_t string_view, sv_condition_fn condition) {
    while (string_view.length && !condition(string_view))
        sv_chop(&string_view);

    if (!string_view.length)
        return (string_view_t){ .start = NULL, .length = 0 };

    return string_view;
}

string_view_t sv_find_if_va(string_view_t string_view, sv_condition_va_fn condition, ...) {
    va_list pargs;
    va_start(pargs, condition);
    while (string_view.length && !condition(string_view, pargs))
        sv_chop(&string_view);
    va_end(pargs);

    if (!string_view.length)
        return (string_view_t){ .start = NULL, .length = 0 };

    return string_view;
}

struct sv_sep_pair sv_split_at(string_view_t string_view, string_view_t separator) {
    string_view_t before = string_view, after = sv_find(string_view, separator);
    sv_chop_end_n(&before, after.length);
    sv_chop_n(&after, separator.length);
    return (struct sv_sep_pair){
        .before = before,
        .after = after,
    };
}

#if 0
struct sv_sep_pair sv_split_at_last(string_view_t string_view, string_view_t separator) {
}
struct sv_sep_pair sv_split_at_n(string_view_t string_view, string_view_t separator, size_t separator_index);
struct sv_sep_pair sv_split_at_any(string_view_t string_view, string_view_t symbols);
struct sv_sep_pair sv_split_at_any_last(string_view_t string_view, string_view_t symbols);
struct sv_sep_pair sv_split_at_any_n(string_view_t string_view, string_view_t symbols, size_t separator_index);
struct sv_sep_pair sv_split_at_symbol(string_view_t string_view, char symbol);
struct sv_sep_pair sv_split_at_symbol_last(string_view_t string_view, char symbol);
struct sv_sep_pair sv_split_at_symbol_n(string_view_t string_view, char symbol, size_t separator_index);

size_t sv_count_symbol(string_view_t string_view, char symbol);
size_t sv_count_nonoverlapping(string_view_t string_view, string_view_t substring);
size_t sv_count_overlapping(string_view_t string_view, string_view_t substring);
#endif
