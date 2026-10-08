#ifndef BANK_H
#define BANK_H

#include <stdbool.h>
#include <stdint.h>

#define NAME_SIZE 80
/* Each account is a node in a singly linked list. Money is stored in cents. */
typedef struct Account {
    int number;
    char name[NAME_SIZE];
    int64_t balance;
    struct Account *next;
} Account;

Account *bank_find(Account *head, int number);
bool bank_create(Account **head, int number, const char *name, int64_t balance);
bool bank_delete(Account **head, int number);
bool bank_deposit(Account *account, int64_t amount);
bool bank_withdraw(Account *account, int64_t amount);
bool bank_transfer(Account *from, Account *to, int64_t amount);
void bank_free(Account *head);
/* Load returns 1 on success, 0 if missing, -1 on invalid data or I/O error. */
int bank_load(Account **head, const char *path);
bool bank_save(const Account *head, const char *path);

#endif
