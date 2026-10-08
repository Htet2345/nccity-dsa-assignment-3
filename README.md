# nccity-dsa-assignment-3

## Bank Management System: Implementation Details

This assignment implements a basic Bank Management System in C11 using a singly linked list. The submission contains the research in [RESEARCH.md](RESEARCH.md), the C program in `src/` and `include/`, and the implementation details in this document.

### Program features

The menu lets the user create accounts, list accounts, search by account number, deposit money, withdraw money, transfer money between accounts, delete zero-balance accounts, save, and exit. The program loads saved accounts at startup and saves after successful changes and on exit.

### Linked-list data structure

Each `Account` node contains:

| Field | Type | Purpose |
| --- | --- | --- |
| `number` | `int` | Unique positive account identifier |
| `name` | `char[80]` | Customer name, up to 79 bytes plus the terminator |
| `balance` | `int64_t` | Nonnegative balance in whole cents |
| `next` | `struct Account *` | Pointer to the next account |

The `head` pointer identifies the first node. An empty bank has `head == NULL`; the last node has `next == NULL`. Nodes are allocated with `malloc` when accounts are created and released with `free` when deleted or when the program exits.

```text
head -> [101 | Alice | 10000 | next] -> [202 | Bob | 0 | NULL]
```

In this example, Alice has a balance of 100.00 and Bob has 0.00.

### Separate source files

| File | Responsibility |
| --- | --- |
| `include/bank.h` | Account structure and declarations for account and storage operations |
| `include/input.h` | Input function declarations |
| `src/main.c` | Menu, operation selection, result messages, and automatic saving |
| `src/bank.c` | Linked-list search, insertion, deletion, transactions, and memory cleanup |
| `src/input.c` | Line input, integer validation, and decimal-to-cents conversion |
| `src/storage.c` | Account file loading and saving |

### Algorithms

**Create:** Validate the number, name, and opening balance. Search the list to reject duplicate numbers. Allocate a node, initialize its fields, and append it to the list.

**Search:** Start at `head` and follow `next` until the account number matches or the end is reached.

**Deposit and withdraw:** Find the account. Reject nonpositive amounts. Deposit checks the balance limit before adding; withdrawal checks available funds before subtracting.

**Transfer:** Find both accounts and reject a missing destination or transfer to the same account. Check available funds and the destination balance limit before changing either balance. Subtract from the source and add to the destination only after all checks pass.

**Delete:** Locate the pointer that refers to the account. Reject deletion if its balance is nonzero. Redirect that pointer to the next node and free the removed node. The same approach handles removal of the head, a middle node, or the tail.

**Cleanup:** Walk through the list, retain each node's next pointer, and free the current node.

| Operation | Time complexity |
| --- | --- |
| Search, create, delete | O(n) |
| List all accounts, save, free | O(n) |
| Deposit/withdraw after finding the account | O(1) |
| Transfer after finding both accounts | O(1) |
| Load n accounts with duplicate checks and appending | O(n²) |

Total account memory usage is O(n). Menu transactions require O(n) overall because account lookup traverses the list.

### Input validation and money representation

The program reads complete lines with `fgets`. It rejects empty input and overlong lines, checks that account numbers are positive integers within the supported range, and prevents duplicate account numbers. Money accepts decimal values with at most two decimal places, such as `125.50`, and is converted directly to whole cents without floating-point arithmetic. Negative balances, zero-value transactions, overdrafts, and integer overflow are rejected.

### File storage

Accounts are stored in `accounts.tsv` in the working directory. Each row contains an account number, balance in cents, and customer name, separated by tabs. Loading builds a temporary linked list and validates the complete file before replacing the current list. A missing file starts a new bank; an invalid or unreadable file causes an error without overwriting it.

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

The project was compiled with warnings treated as errors. Local checks covered duplicate accounts, invalid input, overdrafts, successful transfers, deletion, balance overflow, file reloads, rejection of malformed files, and end-of-input handling. Debugger checks confirmed a breakpoint at `main` and successful execution.

This is a single-user educational simulation. Authentication, encryption, transaction history, interest calculation, and simultaneous access are outside the implementation. Use fictional account information when demonstrating it.
