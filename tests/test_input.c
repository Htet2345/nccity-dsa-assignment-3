#include "input.h"
#include "bank.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *path = "build/test_input.txt";
    FILE *file = fopen(path, "w");
    assert(file);
    for (int i = 0; i < NAME_SIZE - 1; i++) fputc('A', file);
    fputc('\n', file);
    for (int i = 0; i < NAME_SIZE; i++) fputc('B', file);
    fputs("\n\nBad\tName\n  Trimmed Name  \n", file);
    fputs("abc\n-1\n2147483648\n2147483647\n", file);
    fputs("-1\n0\n1.234\n1.\n.50\nabc\n92233720368547758.08\n92233720368547758.07\n", file);
    fputs("0\n0.01\n12.3\n", file);
    fputs("Final value", file);
    assert(fclose(file) == 0);
    assert(freopen(path, "r", stdin));
    char name[NAME_SIZE];
    assert(input_line("", name, sizeof name) && strlen(name) == NAME_SIZE - 1);
    assert(input_line("", name, sizeof name) && strcmp(name, "Trimmed Name") == 0);
    int number;
    assert(input_number("", 1, INT_MAX, &number) && number == INT_MAX);
    int64_t amount;
    assert(input_money("", false, &amount) && amount == INT64_MAX);
    assert(input_money("", true, &amount) && amount == 0);
    assert(input_money("", false, &amount) && amount == 1);
    assert(input_money("", false, &amount) && amount == 1230);
    assert(input_line("", name, sizeof name) && strcmp(name, "Final value") == 0);
    assert(!input_line("", name, sizeof name));
    assert(fclose(stdin) == 0);
    remove(path);
    puts("All input validation tests passed.");
    return 0;
}
