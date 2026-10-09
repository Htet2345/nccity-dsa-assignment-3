#include "bank.h"
#include <stdlib.h>
#include <string.h>

bool bank_history_add(Account *account, TransactionType type, int64_t amount,
                      int64_t balance_after, int other_account) {
    if (!account || type < OPENING_BALANCE || type > TRANSFER_OUT ||
        amount < 0 || balance_after < 0 ||
        (type != OPENING_BALANCE && amount == 0) ||
        ((type == TRANSFER_IN || type == TRANSFER_OUT) ?
         (other_account <= 0 || other_account == account->number) : other_account != 0))
        return false;
    Transaction *entry = malloc(sizeof *entry);
    if (!entry) return false;
    *entry = (Transaction){type, amount, balance_after, other_account, NULL};
    if (account->history_tail) account->history_tail->next = entry;
    else account->history = entry;
    account->history_tail = entry;
    return true;
}

void bank_history_clear(Account *account) {
    while (account->history) {
        Transaction *next = account->history->next;
        free(account->history);
        account->history = next;
    }
    account->history_tail = NULL;
}

bool bank_update(Account *account, const char *name) {
    if (!account || !name || !*name || strlen(name) >= NAME_SIZE ||
        strpbrk(name, "\t\r\n")) return false;
    memmove(account->name, name, strlen(name) + 1);
    return true;
}

Account *bank_find(Account *head, int number) {
    for (; head; head = head->next)
        if (head->number == number) return head;
    return NULL;
}

bool bank_create(Account **head, int number, const char *name, int64_t balance) {
    if (!head || number <= 0 || !name || !*name || strlen(name) >= NAME_SIZE ||
        strpbrk(name, "\t\r\n") || balance < 0 || bank_find(*head, number)) return false;
    Account *node = malloc(sizeof *node);
    if (!node) return false;
    node->number = number;
    strcpy(node->name, name);
    node->balance = balance;
    node->history = node->history_tail = NULL;
    if (!bank_history_add(node, OPENING_BALANCE, balance, balance, 0)) {
        free(node);
        return false;
    }
    node->next = NULL;
    /* Append so the display preserves account creation order. */
    Account **tail = head;
    while (*tail) tail = &(*tail)->next;
    *tail = node;
    return true;
}

bool bank_delete(Account **head, int number) {
    if (!head) return false;
    while (*head && (*head)->number != number) head = &(*head)->next;
    if (!*head || (*head)->balance != 0) return false;
    Account *removed = *head;
    *head = removed->next;
    bank_history_clear(removed);
    free(removed);
    return true;
}

bool bank_deposit(Account *account, int64_t amount) {
    if (!account || amount <= 0 || account->balance > INT64_MAX - amount) return false;
    if (!bank_history_add(account, DEPOSIT, amount, account->balance + amount, 0)) return false;
    account->balance += amount;
    return true;
}

bool bank_withdraw(Account *account, int64_t amount) {
    if (!account || amount <= 0 || account->balance < amount) return false;
    if (!bank_history_add(account, WITHDRAWAL, amount, account->balance - amount, 0)) return false;
    account->balance -= amount;
    return true;
}

bool bank_transfer(Account *from, Account *to, int64_t amount) {
    if (!from || !to || from == to || amount <= 0 || from->balance < amount ||
        to->balance > INT64_MAX - amount) return false;
    /* Allocate both records before changing either balance. */
    Transaction *out = malloc(sizeof *out);
    Transaction *in = malloc(sizeof *in);
    if (!out || !in) { free(out); free(in); return false; }
    *out = (Transaction){TRANSFER_OUT, amount, from->balance - amount, to->number, NULL};
    *in = (Transaction){TRANSFER_IN, amount, to->balance + amount, from->number, NULL};
    if (from->history_tail) from->history_tail->next = out;
    else from->history = out;
    from->history_tail = out;
    if (to->history_tail) to->history_tail->next = in;
    else to->history = in;
    to->history_tail = in;
    from->balance -= amount;
    to->balance += amount;
    return true;
}

void bank_free(Account *head) {
    while (head) {
        Account *next = head->next;
        bank_history_clear(head);
        free(head);
        head = next;
    }
}
