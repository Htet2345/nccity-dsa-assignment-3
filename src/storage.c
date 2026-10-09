#include "bank.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

/* History rows follow their account: H, type, amount, balance, other account. */
static bool load_history(Account *account, char *line) {
    intmax_t values[4];
    char *field = line + 2;
    for (int i = 0; i < 4; i++) {
        char *end;
        errno = 0;
        values[i] = strtoimax(field, &end, 10);
        if (errno || end == field || (i < 3 ? *end != '\t' : *end != '\0')) return false;
        field = end + (i < 3 ? 1 : 0);
    }
    if (!account || values[0] < OPENING_BALANCE || values[0] > TRANSFER_OUT ||
        values[1] < 0 || values[1] > INT64_MAX || values[2] < 0 || values[2] > INT64_MAX ||
        values[3] < 0 || values[3] > INT32_MAX) return false;
    TransactionType type = (TransactionType)values[0];
    int64_t amount = (int64_t)values[1], balance = (int64_t)values[2];
    bool credit = type == DEPOSIT || type == TRANSFER_IN;
    if (type == OPENING_BALANCE) {
        if (account->history || amount != balance) return false;
    } else {
        if ((credit && balance < amount) || (!credit && balance > INT64_MAX - amount)) return false;
        int64_t before = credit ? balance - amount : balance + amount;
        if (account->history_tail && account->history_tail->balance_after != before) return false;
    }
    return bank_history_add(account, type, amount, balance, (int)values[3]);
}

int bank_load(Account **head, const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) return errno == ENOENT ? 0 : -1;
    Account *loaded = NULL;
    Account *current = NULL;
    char line[256];
    bool valid = true;
    while (fgets(line, sizeof line, file)) {
        if (!strchr(line, '\n') && !feof(file)) { valid = false; break; }
        line[strcspn(line, "\r\n")] = '\0';
        if (strncmp(line, "H\t", 2) == 0) {
            if (!load_history(current, line)) { valid = false; break; }
            continue;
        }
        if (current && current->history_tail && current->history_tail->balance_after != current->balance) {
            valid = false; break;
        }
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
        current = bank_find(loaded, (int)number);
        /* Old files have no history; do not invent past transactions. */
        bank_history_clear(current);
    }
    if (current && current->history_tail && current->history_tail->balance_after != current->balance) valid = false;
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
        for (const Transaction *entry = head->history; entry; entry = entry->next) {
            if (fprintf(file, "H\t%d\t%" PRId64 "\t%" PRId64 "\t%d\n",
                        (int)entry->type, entry->amount, entry->balance_after, entry->other_account) < 0) {
                success = false;
                break;
            }
        }
        if (!success) break;
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
