#include "input.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool input_line(const char *prompt, char *buffer, size_t size) {
    for (;;) {
        fputs(prompt, stdout);
        fflush(stdout);
        if (!fgets(buffer, (int)size, stdin)) return false;
        char *newline = strchr(buffer, '\n');
        if (!newline && !feof(stdin)) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            puts("Input is too long. Please try again.");
            continue;
        }
        buffer[strcspn(buffer, "\r\n")] = '\0';
        size_t start = 0;
        while (isspace((unsigned char)buffer[start])) start++;
        memmove(buffer, buffer + start, strlen(buffer + start) + 1);
        size_t length = strlen(buffer);
        while (length && isspace((unsigned char)buffer[length - 1])) buffer[--length] = '\0';
        if (*buffer && !strchr(buffer, '\t')) return true;
        puts("Please enter a nonempty value without tabs.");
    }
}

bool input_number(const char *prompt, int minimum, int maximum, int *value) {
    char line[100];
    while (input_line(prompt, line, sizeof line)) {
        char *end;
        errno = 0;
        long parsed = strtol(line, &end, 10);
        if (!errno && !*end && parsed >= minimum && parsed <= maximum) {
            *value = (int)parsed;
            return true;
        }
        printf("Enter a whole number from %d to %d.\n", minimum, maximum);
    }
    return false;
}

bool input_money(const char *prompt, bool allow_zero, int64_t *value) {
    char line[100];
    while (input_line(prompt, line, sizeof line)) {
        const char *p = line;
        int64_t units = 0;
        bool valid = isdigit((unsigned char)*p) != 0;
        while (isdigit((unsigned char)*p)) {
            int digit = *p++ - '0';
            if (units > (INT64_MAX / 100 - digit) / 10) { valid = false; break; }
            units = units * 10 + digit;
        }
        int cents = 0;
        if (*p == '.') {
            p++;
            if (!isdigit((unsigned char)*p)) valid = false;
            else {
                cents = (*p++ - '0') * 10;
                if (isdigit((unsigned char)*p)) cents += *p++ - '0';
            }
        }
        if (*p || units > (INT64_MAX - cents) / 100) valid = false;
        if (valid) {
            int64_t amount = units * 100 + cents;
            if (allow_zero || amount > 0) { *value = amount; return true; }
        }
        puts("Enter a valid amount with at most two decimal places (for example 125.50).");
    }
    return false;
}
