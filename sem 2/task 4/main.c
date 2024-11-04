#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "patch.h"

#define BUF_SIZE (100 + 1)

bool read_input(char *buffer, size_t cap, size_t *len);
bool check_ends_with_unique_cap(const char *text, size_t length);

int main(void) {
	// Handle input
	char text[BUF_SIZE];
	size_t length;
	printf("Please enter string: ");
	if (!read_input(text, BUF_SIZE, &length)) return 1;
	if (!length) return 2;
	printf("%s\n", text);

	// Check
	bool satisfy = check_ends_with_unique_cap(text, length);
	printf("Uses rule %d\n", 2 - satisfy);

	// Transform
	if (satisfy) transform_caps(text);
	else leave_unique(text);

	// Handle output
	printf("%s\n", text);

	return 0;
}

bool read_input(char *buffer, size_t cap, size_t *len) {
	if (fgets(buffer, cap, stdin) == NULL)
		return false;
	size_t length = strlen(buffer);
	if (buffer[length - 1] == '\n') {
		buffer[length - 1] = '\0';
		length--;
	}
	if (len)
		*len = length;
	return true;
}

bool check_ends_with_unique_cap(const char *text, size_t length) {
	char last = text[length - 1];
	if (last < 'A' || last > 'Z')
		return false;
	// Find first occurence of `last` and check that it's followed by '\0'
	while (*text++ != last);
	return !*text;
}
