#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef unsigned long long ull;

char *read_num(char *view, ull *num) {
    while (*view != '\0' && ('0' > *view || *view > '9'))
        view++;
    if (!view) return NULL;
    if (*view == '\0') return NULL;
    ull result = 0;
    while ('0' <= *view && *view <= '9') {
        result = result * 10 + *view - '0';
        view++;
    }
    if (num) *num = result;
    return view;
}

int main(int argc, char *argv[]) {
    const unsigned files_count = argc - 1;
    ull **numbers = malloc(sizeof(*numbers) * files_count);
    size_t *lengths = malloc(sizeof(*lengths) * files_count);
    for (unsigned i = 0; i < files_count; i++) {
        FILE *file = fopen(argv[i + 1], "r+");
        fseek(file, 0, SEEK_END);
        ull length = ftell(file);
        char *nums = malloc(sizeof(*nums) * (length + 1));
        nums[length] = '\0';
        fseek(file, 0, SEEK_SET);
        fread(nums, 1, length, file);
        fclose(file);
        char *cur = nums;
        size_t cap = 1;
        numbers[i] = malloc(sizeof(*numbers[i]) * cap);
        ull num;
        size_t len = 0;
        while ((cur = read_num(cur, &num))) {
            if (len == cap) {
                cap *= 2;
                numbers[i] = realloc(numbers[i], sizeof(*numbers[i]) * cap);
            }
            numbers[i][len++] = num;
        }
        lengths[i] = len;
        free(nums);
    }
    bool ended = false;
    size_t *indicies = calloc(files_count, sizeof(*indicies));
    while (!ended) {
        ended = true;
        size_t j = files_count;
        ull min = (ull)-1;
        for (size_t i = 0; i < files_count; i++) {
            if (indicies[i] != lengths[i]) {
                if (min > numbers[i][indicies[i]]) {
                    j = i;
                    min = numbers[i][indicies[i]];
                    ended = false;
                }
            }
        }
        if (j != files_count) {
            printf("%llu\n", min);
            indicies[j]++;
        }
    }
    free(indicies);
    free(lengths);
    for (size_t i = 0; i < files_count; i++) free(numbers[i]);
    free(numbers);
}