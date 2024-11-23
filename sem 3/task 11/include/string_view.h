#ifndef STRING_VIEW_H
#define STRING_VIEW_H 1

/* I've written this library a while ago ans simplyy copypasted it */

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#define SV_FROM_CSTR(_STRING) ((string_view_t){ .start = (_STRING), .length = sizeof(_STRING) - 1 })
#define SV_PRINTF "%.*s"
#define SV_PRINTF_ARG(_STRING_VIEW) (_STRING_VIEW).length, (_STRING_VIEW).start

typedef struct {
	const char *start;
	size_t length;
} string_view_t;

typedef bool (*sv_condition_fn)(string_view_t string_view);
typedef bool (*sv_condition_va_fn)(string_view_t string_view, va_list args); // Shouldn't call va_start & va_end
typedef bool (*sv_chop_condition_fn)(string_view_t string_view, size_t *count);
typedef bool (*sv_chop_condition_va_fn)(string_view_t string_view, size_t *count, va_list args); // Shouldn't call va_start & va_end

string_view_t sv_from_cstr(const char *string);
char *sv_to_cstr(string_view_t string_view);
void sv_to_cstr_at(string_view_t string_view, char buffer[]);

void sv_chop(string_view_t *string_view);
void sv_chop_n(string_view_t *string_view, size_t symbols_count);
void sv_chop_if(string_view_t *string_view, sv_chop_condition_fn condition);
void sv_chop_while(string_view_t *string_view, sv_chop_condition_fn condition);
void sv_chop_end(string_view_t *string_view);
void sv_chop_end_n(string_view_t *string_view, size_t symbols_count);
void sv_chop_end_if(string_view_t *string_view, sv_chop_condition_fn condition);
void sv_chop_end_while(string_view_t *string_view, sv_chop_condition_fn condition);

void sv_chop_if_va(string_view_t *string_view, sv_chop_condition_va_fn condition, ...);
void sv_chop_while_va(string_view_t *string_view, sv_chop_condition_va_fn condition, ...);
void sv_chop_end_while_va(string_view_t *string_view, sv_chop_condition_va_fn condition, ...);
void sv_chop_end_if_va(string_view_t *string_view, sv_chop_condition_va_fn condition, ...);

void sv_trim(string_view_t *string_view);
void sv_trim_any(string_view_t *string_view, string_view_t symbols);

bool sv_starts_with(string_view_t string_view, string_view_t prefix);
bool sv_ends_with(string_view_t string_view, string_view_t suffix);
bool sv_contains(string_view_t string_view, string_view_t substring);
bool sv_empty(string_view_t string_view);
bool sv_is_subview(string_view_t string_view, string_view_t subview);

string_view_t sv_find(string_view_t string_view, string_view_t prefix);
string_view_t sv_find_last(string_view_t string_view, string_view_t prefix);
string_view_t sv_find_symbol(string_view_t string_view, char symbol);
string_view_t sv_find_symbol_last(string_view_t string_view, char symbol);
string_view_t sv_find_if(string_view_t string_view, sv_condition_fn condition);
string_view_t sv_find_if_va(string_view_t string_view, sv_condition_va_fn condition, ...);

struct sv_sep_pair { string_view_t before, after; };
struct sv_sep_pair sv_split_at(string_view_t string_view, string_view_t separator);
#if 0
struct sv_sep_pair sv_split_at_last(string_view_t string_view, string_view_t separator);
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

#endif
