#include "bank.h"
#include "input.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>

static void display(const Account *account) {
    printf("%-12d %-32s %" PRId64 ".%02" PRId64 "\n", account->number,
           account->name, account->balance / 100, account->balance % 100);
}

static void display_history(const Account *account) {
    const char *types[] = {"Opening balance", "Deposit", "Withdrawal", "Transfer in", "Transfer out"};
    display(account);
    if (!account->history) { puts("No recorded transactions for this account."); return; }
    puts("No.  Transaction          Amount          Balance after   Other account");
    size_t index = 1;
    for (const Transaction *entry = account->history; entry; entry = entry->next) {
        printf("%-4zu %-20s %" PRId64 ".%02" PRId64 "          %" PRId64 ".%02" PRId64,
               index++, types[entry->type], entry->amount / 100, entry->amount % 100,
               entry->balance_after / 100, entry->balance_after % 100);
        if (entry->other_account) printf("          %d", entry->other_account);
        putchar('\n');
    }
}

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : "accounts.tsv";
    Account *head = NULL;
    int loaded = bank_load(&head, path);
    if (loaded < 0) {
        fprintf(stderr, "Cannot read account file '%s': invalid data or I/O error.\n", path);
        return 1;
    }
    puts("BANK MANAGEMENT SYSTEM\nAccounts stored using a singly linked list.");
    if (!loaded) puts("No saved accounts found. Starting a new bank.");
    for (;;) {
        puts("\n1. Create account\n2. List accounts\n3. Search account\n4. Deposit"
             "\n5. Withdraw\n6. Transfer\n7. Delete account\n8. Save"
             "\n9. Update Account Information\n10. View Transaction History\n0. Save and exit");
        int choice, number;
        int64_t amount;
        if (!input_number("Choice: ", 0, 10, &choice)) break;
        if (choice == 0) break;
        if (choice == 8) {
            puts(bank_save(head, path) ? "Accounts saved." : "Save failed. Data remains in memory; retry before exiting.");
            continue;
        }
        if (choice == 2) {
            puts("Account      Customer                         Balance");
            if (!head) puts("No accounts yet.");
            for (Account *node = head; node; node = node->next) display(node);
            continue;
        }
        if (!input_number("Account number: ", 1, INT_MAX, &number)) break;
        Account *account = bank_find(head, number);
        bool changed = false;
        if (choice == 1) {
            if (account) { puts("Account number already exists."); continue; }
            char name[NAME_SIZE];
            if (!input_line("Customer name: ", name, sizeof name) ||
                !input_money("Opening balance: ", true, &amount)) break;
            changed = bank_create(&head, number, name, amount);
            puts(changed ? "Account created." : "Could not create account.");
        } else {
            if (!account) { puts("Account not found."); continue; }
            if (choice == 3) { display(account); continue; }
            if (choice == 10) { display_history(account); continue; }
            if (choice == 9) {
                char name[NAME_SIZE];
                display(account);
                if (!input_line("New customer name: ", name, sizeof name)) break;
                changed = bank_update(account, name);
                puts(changed ? "Account information updated." : "Could not update account information.");
            } else if (choice == 7) {
                changed = bank_delete(&head, number);
                puts(changed ? "Account deleted." : "Withdraw or transfer the remaining balance before deletion.");
            } else {
                Account *target = NULL;
                if (choice == 6) {
                    int destination;
                    if (!input_number("Destination account number: ", 1, INT_MAX, &destination)) break;
                    target = bank_find(head, destination);
                    if (!target || target == account) { puts("Choose a different, existing destination account."); continue; }
                }
                if (!input_money("Amount: ", false, &amount)) break;
                if (choice == 4) changed = bank_deposit(account, amount);
                if (choice == 5) changed = bank_withdraw(account, amount);
                if (choice == 6) changed = bank_transfer(account, target, amount);
                puts(changed ? "Transaction completed." : "Transaction refused: insufficient funds, balance limit exceeded, or memory unavailable.");
                display(account);
            }
        }
        if (changed && !bank_save(head, path))
            puts("Save failed. Changes remain in memory; use Save to retry before exiting.");
    }
    bool saved = bank_save(head, path);
    puts(saved ? "Accounts saved. Goodbye!" : "Save failed. Unsaved changes will be lost when this process exits.");
    bank_free(head);
    return saved ? 0 : 1;
}
