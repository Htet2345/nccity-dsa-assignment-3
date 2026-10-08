#include "bank.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

int bank_load(Account **head, const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) return errno == ENOENT ? 0 : -1;
    Account *loaded = NULL;
    char line[256];
    bool valid = true;
    while (fgets(line, sizeof line, file)) {
        if (!strchr(line, '\n') && !feof(file)) { valid = false; break; }
        line[strcspn(line, "\r\n")] = '\0';
        char *first = strchr(line, '\t');
        char *second = first ? strchr(first + 1, '\t') : NULL;
        if (!first || !second) { valid = false; break; }
        *first++ = '\0';
        *second++ = '\0';
        char *end;
        errno = 0;
        long number = strtol(line, &end, 10);
        if (errno || end == line || *end || number <= 0 || number > INT32_MAX) { valid = false; break; }
        errno = 0;
        intmax_t balance = strtoimax(first, &end, 10);
        if (errno || end == first || *end || balance < 0 || balance > INT64_MAX ||
            !bank_create(&loaded, (int)number, second, (int64_t)balance)) { valid = false; break; }
    }
    if (ferror(file)) valid = false;
    if (fclose(file)) valid = false;
    if (!valid) { bank_free(loaded); return -1; }
    bank_free(*head);
    *head = loaded;
    return 1;
}

bool bank_save(const Account *head, const char *path) {
    char temporary[512];
    int length = snprintf(temporary, sizeof temporary, "%s.tmp", path);
    if (length < 0 || (size_t)length >= sizeof temporary) return false;
    FILE *file = fopen(temporary, "w");
    if (!file) return false;
    bool success = true;
    for (; head; head = head->next) {
        if (fprintf(file, "%d\t%" PRId64 "\t%s\n", head->number, head->balance, head->name) < 0) {
            success = false;
            break;
        }
    }
    if (fclose(file)) success = false;
    if (success) {
#ifdef _WIN32
        success = MoveFileExA(temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
        success = rename(temporary, path) == 0;
#endif
    }
    if (!success) remove(temporary);
    return success;
}
