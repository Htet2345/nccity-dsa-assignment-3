#ifndef INPUT_H
#define INPUT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
bool input_line(const char *prompt, char *buffer, size_t size);
bool input_number(const char *prompt, int minimum, int maximum, int *value);
bool input_money(const char *prompt, bool allow_zero, int64_t *value);
#endif
