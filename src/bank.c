#include "bank.h"
#include <stdlib.h>
#include <string.h>

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
    free(removed);
    return true;
}

bool bank_deposit(Account *account, int64_t amount) {
    if (!account || amount <= 0 || account->balance > INT64_MAX - amount) return false;
    account->balance += amount;
    return true;
}

bool bank_withdraw(Account *account, int64_t amount) {
    if (!account || amount <= 0 || account->balance < amount) return false;
    account->balance -= amount;
    return true;
}

bool bank_transfer(Account *from, Account *to, int64_t amount) {
    if (!from || !to || from == to || amount <= 0 || from->balance < amount ||
        to->balance > INT64_MAX - amount) return false;
    from->balance -= amount;
    to->balance += amount;
    return true;
}

void bank_free(Account *head) {
    while (head) {
        Account *next = head->next;
        free(head);
        head = next;
    }
}
