#include "bank.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *path = "build/test_accounts.tsv";
    Account *head = NULL;
    assert(bank_create(&head, 1, "Alice Example", 10000));
    assert(bank_create(&head, 2, "Bob Example", 0));
    assert(bank_create(&head, 3, "Carol Example", 0));
    assert(!bank_create(&head, 1, "Duplicate", 0));
    assert(!bank_create(&head, -1, "Invalid", 0));
    assert(!bank_create(&head, 4, "Bad\tName", 0));
    assert(!bank_create(&head, 4, "Negative", -1));
    Account *alice = bank_find(head, 1);
    Account *bob = bank_find(head, 2);
    assert(bank_update(alice, "Alice Updated"));
    assert(strcmp(alice->name, "Alice Updated") == 0);
    assert(alice->number == 1 && alice->balance == 10000);
    assert(!bank_update(NULL, "Nobody"));
    assert(!bank_update(alice, ""));
    assert(!bank_update(alice, "Bad\tName"));
    char long_name[NAME_SIZE + 1];
    memset(long_name, 'x', NAME_SIZE);
    long_name[NAME_SIZE] = '\0';
    assert(!bank_update(alice, long_name));
    assert(strcmp(alice->name, "Alice Updated") == 0);
    assert(alice->history == alice->history_tail);
    assert(alice->history->type == OPENING_BALANCE && alice->history->amount == 10000);
    assert(!bank_withdraw(alice, 10001));
    assert(!bank_deposit(NULL, 100));
    assert(!bank_withdraw(NULL, 100));
    assert(!bank_transfer(alice, NULL, 100));
    assert(!bank_transfer(NULL, bob, 100));
    assert(!bank_deposit(alice, -1));
    assert(!bank_withdraw(alice, -1));
    assert(!bank_transfer(alice, bob, -1));
    assert(!bank_transfer(alice, bob, 10001));
    assert(!bank_delete(&head, 999));
    assert(!bank_deposit(alice, 0));
    assert(!bank_transfer(alice, alice, 100));
    assert(alice->history == alice->history_tail); /* Failed operations add no records. */
    assert(bank_transfer(alice, bob, 2500));
    assert(alice->balance == 7500 && bob->balance == 2500);
    assert(alice->history_tail->type == TRANSFER_OUT && alice->history_tail->other_account == 2);
    assert(bob->history_tail->type == TRANSFER_IN && bob->history_tail->other_account == 1);
    assert(alice->history_tail->amount == 2500 && alice->history_tail->balance_after == 7500);
    assert(!bank_delete(&head, 1));
    assert(bank_deposit(alice, 125));
    assert(bank_save(head, path));
    Account *copy = NULL;
    assert(bank_load(&copy, path) == 1);
    assert(bank_find(copy, 1)->balance == 7625);
    assert(strcmp(bank_find(copy, 1)->name, "Alice Updated") == 0);
    const Transaction *entry = bank_find(copy, 1)->history;
    assert(entry->type == OPENING_BALANCE);
    entry = entry->next;
    assert(entry->type == TRANSFER_OUT && entry->amount == 2500 && entry->other_account == 2);
    entry = entry->next;
    assert(entry->type == DEPOSIT && entry->balance_after == 7625 && entry->next == NULL);
    assert(bank_find(copy, 1)->history_tail == entry);
    assert(strcmp(bank_find(copy, 2)->name, "Bob Example") == 0);
    assert(bank_delete(&copy, 3)); /* Tail removal. */
    assert(bank_withdraw(bank_find(copy, 2), 2500));
    assert(bank_find(copy, 2)->history_tail->type == WITHDRAWAL);
    assert(bank_delete(&copy, 2)); /* Remaining tail. */
    assert(bank_withdraw(bank_find(copy, 1), 7625));
    assert(bank_delete(&copy, 1)); /* Head/last node. */
    assert(copy == NULL);
    assert(bank_create(&copy, 4, "Limit", INT64_MAX));
    assert(!bank_deposit(copy, 1));
    assert(!bank_transfer(alice, copy, 1));
    assert(alice->balance == 7625);
    assert(!bank_save(head, "build/nonexistent-directory/accounts.tsv"));
    FILE *file = fopen(path, "w");
    assert(file);
    fputs("1\t100\tValid\ninvalid data\n", file);
    assert(fclose(file) == 0);
    assert(bank_load(&copy, path) == -1);
    assert(copy->number == 4); /* Invalid load leaves original list intact. */
    file = fopen(path, "w");
    assert(file);
    fputs("5\t100\tLegacy Account\n", file);
    assert(fclose(file) == 0);
    assert(bank_load(&copy, path) == 1);
    assert(copy->history == NULL && copy->history_tail == NULL);
    assert(bank_deposit(copy, 50));
    assert(bank_save(copy, path));
    assert(bank_load(&copy, path) == 1);
    assert(copy->history->type == DEPOSIT && copy->history->balance_after == 150);
    const char *invalid_history[] = {
        "H\t1\t50\t150\t0\n",
        "5\t150\tLegacy\nH\t9\t50\t150\t0\n",
        "5\t150\tLegacy\nH\t1\t0\t150\t0\n",
        "5\t150\tLegacy\nH\t1\t50\t140\t0\n",
        "5\t150\tLegacy\nH\t1\t50\t150\t0\nH\t2\t10\t130\t0\n",
        "5\t150\tLegacy\nH\t3\t50\t150\t5\n",
        "5\t150\tLegacy\nH\t1\t9223372036854775808\t150\t0\n"
    };
    for (size_t i = 0; i < sizeof invalid_history / sizeof invalid_history[0]; i++) {
        file = fopen(path, "w");
        assert(file);
        fputs(invalid_history[i], file);
        assert(fclose(file) == 0);
        assert(bank_load(&copy, path) == -1);
        assert(copy->number == 5 && copy->balance == 150 && copy->history->type == DEPOSIT);
    }
    bank_free(head);
    bank_free(copy);
    assert(bank_save(NULL, path));
    copy = NULL;
    assert(bank_load(&copy, path) == 1 && copy == NULL);
    remove(path);
    assert(bank_load(&copy, path) == 0);
    puts("All bank and storage tests passed.");
    return 0;
}
