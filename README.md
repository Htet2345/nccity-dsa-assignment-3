# nccity-dsa-assignment-3

## Bank Management System: Implementation Details

This assignment implements a basic Bank Management System in C11 using a singly linked list. The submission contains the research in [RESEARCH.md](RESEARCH.md), the C program in `src/` and `include/`, and the implementation details in this document.

### Program features

The menu lets the user create accounts, list accounts, search by account number, deposit money, withdraw money, transfer money between accounts, delete zero-balance accounts, update account information, view transaction history, save, and exit. The program loads saved accounts and history at startup and saves after successful changes and on exit.

Choose **9. Update Account Information**, enter an existing account number, and enter the new customer name. The account number stays fixed as its unique identifier, and the balance changes only through money operations. Choose **10. View Transaction History** and enter an account number to see its recorded transactions in chronological order, including the type, amount, balance after the transaction, and the other account number for transfers.

### Linked-list data structure

Each `Account` node contains:

| Field | Type | Purpose |
| --- | --- | --- |
| `number` | `int` | Unique positive account identifier |
| `name` | `char[80]` | Customer name, up to 79 bytes plus the terminator |
| `balance` | `int64_t` | Nonnegative balance in whole cents |
| `history` | `Transaction *` | First transaction in this account's history |
| `history_tail` | `Transaction *` | Last transaction, allowing O(1) appends |
| `next` | `struct Account *` | Pointer to the next account |

The `head` pointer identifies the first node. An empty bank has `head == NULL`; the last node has `next == NULL`. Nodes are allocated with `malloc` when accounts are created and released with `free` when deleted or when the program exits.

```text
head -> [101 | Alice | 10000 | next] -> [202 | Bob | 0 | NULL]
```

In this example, Alice has a balance of 100.00 and Bob has 0.00.

Each account also owns a singly linked list of `Transaction` nodes. A node holds its transaction type, amount in cents, balance after the transaction, the other account number (or zero when not applicable), and a pointer to the next record. Newly created accounts record their opening balance, including a zero opening balance. Failed operations do not add records. History is shown in insertion order; timestamps are not stored.

### Separate source files

| File | Responsibility |
| --- | --- |
| `include/bank.h` | Account structure and declarations for account and storage operations |
| `include/input.h` | Input function declarations |
| `src/main.c` | Menu, operation selection, result messages, and automatic saving |
| `src/bank.c` | Linked-list search, insertion, deletion, transactions, and memory cleanup |
| `src/input.c` | Line input, integer validation, and decimal-to-cents conversion |
| `src/storage.c` | Account and transaction history loading and saving |

### Algorithms

**Create:** Validate the number, name, and opening balance. Search the list to reject duplicate numbers. Allocate a node, initialize its fields, and append it to the list.

**Search:** Start at `head` and follow `next` until the account number matches or the end is reached.

**Deposit and withdraw:** Find the account. Reject nonpositive amounts. Deposit checks the balance limit before adding; withdrawal checks available funds before subtracting.

**Transfer:** Find both accounts and reject a missing destination or transfer to the same account. Check available funds and the destination balance limit before changing either balance. Subtract from the source and add to the destination only after all checks pass.

**Update account information:** Find the account, validate the replacement customer name, and copy it into the existing node. Empty names, names containing tabs or line breaks, and names exceeding the field capacity are rejected.

**Transaction history:** Allocate a record before applying a deposit or withdrawal. Transfers allocate both outgoing and incoming records before changing either account, so allocation failure leaves both balances and histories intact. Append successful records using each account's tail pointer. To view history, traverse the selected account's records from oldest to newest.

**Delete:** Locate the pointer that refers to the account. Reject deletion if its balance is nonzero. Redirect that pointer to the next node and free its history and the removed node. The same approach handles removal of the head, a middle node, or the tail. Deleting an account removes its history; surviving accounts retain their transfer records referencing its number.

**Cleanup:** Walk through the list, retain each node's next pointer, free its transaction history, and free the current node.

| Operation | Time complexity |
| --- | --- |
| Search, create, delete | O(n) |
| List all accounts | O(n) |
| Update account information including lookup | O(n) |
| View history for an account with k records, including lookup | O(n + k) |
| Save, free all accounts and t transaction records | O(n + t) |
| Deposit/withdraw after finding the account | O(1) |
| Transfer after finding both accounts | O(1) |
| Load n accounts with duplicate checks and t transaction records | O(n² + t) |

Total memory usage is O(n + t). Menu transactions require O(n) overall because account lookup traverses the list. Deleting an account also takes O(k) to free its history.

### Input validation and money representation

The program reads complete lines with `fgets`. It rejects empty input and overlong lines, checks that account numbers are positive integers within the supported range, and prevents duplicate account numbers. Money accepts decimal values with at most two decimal places, such as `125.50`, and is converted directly to whole cents without floating-point arithmetic. Negative balances, zero-value transactions, overdrafts, and integer overflow are rejected.

### File storage

Accounts are stored in `accounts.tsv` in the working directory. Each row contains an account number, balance in cents, and customer name, separated by tabs. Loading builds a temporary linked list and validates the complete file before replacing the current list. A missing file starts a new bank; an invalid or unreadable file causes an error without overwriting it.

Transaction rows follow their owning account row in the same file. Their format is `H`, transaction type (`0` opening balance, `1` deposit, `2` withdrawal, `3` transfer in, `4` transfer out), amount in cents, balance after the transaction, and other account number, separated by tabs. Loading validates the history fields, consecutive balances, and the final recorded balance against the account balance. Accounts and histories are saved together through the temporary file.

Older account files without history rows still load. They show no recorded transactions until a new money operation succeeds; historical activity cannot be reconstructed from a balance alone.

Saved data (`accounts.tsv` and its temporary file) is excluded from Git. A fresh GitHub clone starts with no accounts. Later runs restore accounts created locally. To reset a local demonstration, exit the program and delete `accounts.tsv` before running it again.

Saving writes a temporary file and replaces the previous file after a successful write. Windows uses `MoveFileExA`; other platforms use `rename`. Save errors are shown to the user. Data still in memory can be saved again while the menu is running; unsaved changes cannot survive process termination.

### Compile and run

With GCC available, run the following from the repository root:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude src/main.c src/bank.c src/input.c src/storage.c -o bank
```

On Windows, run `./bank.exe`; on Linux or macOS, run `./bank`. An optional first argument specifies another account file, for example `./bank.exe demo.tsv`. Its parent directory must already exist.

### Example operation

Create account 101 with 100.00 and account 202 with 0.00. Deposit 20.00 into account 101, then transfer 30.00 to account 202. The balances become 90.00 and 30.00. A withdrawal of 100.00 from account 101 is rejected because it exceeds the available funds. Account deletion is allowed only after its balance becomes zero.

### Verification and scope

On Windows, run `powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Test` to compile and execute bank/storage tests, input boundary tests, and console workflow tests. These use temporary files in `build/`, preserving your normal saved accounts.

The project was compiled with warnings treated as errors. Local checks covered duplicate accounts, invalid input, overdrafts, successful transfers, deletion, balance overflow, account name updates, transaction history ordering, failed operations leaving history unchanged, history reloads, legacy account files, rejection of malformed files, and end-of-input handling. Console checks exercised both new menu options and reopening saved history.

This is a single-user educational simulation. Authentication, encryption, interest calculation, and simultaneous access are outside the implementation. Use fictional account information when demonstrating it.
