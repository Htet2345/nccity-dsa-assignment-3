# Bank Management System — DSA Assignment 3

A console application written in C11. It uses a **singly linked list** to store accounts in memory, with separate source files for account operations, input handling, persistence, and the menu.

## Features

- Create accounts with unique positive account numbers and customer names.
- List or search accounts; deposit, withdraw, and transfer money.
- Reject duplicate accounts, overdrafts, invalid amounts, and balance overflow.
- Delete accounts after their balance reaches zero.
- Update customer names and view transaction histories.
- Save after changes and on exit; restore accounts when the program starts.
- Save histories with accounts; a fresh clone starts with no accounts.
- Store money in integer cents to avoid floating-point rounding errors.

## Run in VS Code on Windows

Open this entire folder in VS Code. The Microsoft C/C++ extension is recommended (already present on the current computer).

The project uses a portable [w64devkit GCC/GDB toolchain](https://github.com/skeeto/w64devkit/releases/tag/v2.10.0) inside `.tools`. No permanent PATH changes are needed. On another Windows computer, or if `.tools` is missing, run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\setup.ps1
```

Setup downloads the official x64 toolchain, verifies its SHA-256 checksum, and extracts it into this folder. Internet access is required for the download. The toolchain is excluded from Git.

1. Press **Ctrl+Shift+B** to build.
2. Select **Terminal → Run Task → Run bank** to use the menu in the terminal.
3. Press **F5** and choose **Debug bank** to debug. A separate console opens for menu input; breakpoints work in the C source files.

Or build and run with one terminal command:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\run.ps1
```

Run the automated account, storage, input, and console workflow tests:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Test
```

The executable is `build/bank.exe`. Accounts are stored in `accounts.tsv` in the current working directory. You can supply a different file: `./build/bank.exe ./my-accounts.tsv`. Keep the parent directory of that file present.

## Structure and algorithms

| File | Responsibility |
| --- | --- |
| `include/bank.h` | Account node and operation declarations |
| `src/bank.c` | Linked-list insertion, search, deletion, money operations, cleanup |
| `src/input.c` | Validated line, integer, and decimal money input |
| `src/storage.c` | Load/save tab-separated account records |
| `src/main.c` | Interactive menu |
| `scripts/` | Compiler environment setup, build, and run |
| `tests/test_bank.c` | Account and persistence regression checks |
| `tests/test_input.c` | Input validation and boundary checks |
| `tests/test_console.ps1` | Menu workflows, reloads, and empty startup checks |

Each account stores its number, customer name, balance, history pointers, and pointer to the next account. Lookup takes O(n); deposit/withdraw and history appends take O(1) after finding the account. Deletion frees the account and its history. Loading n accounts and t transaction records takes O(n² + t). Memory usage is O(n + t).

Each saved row contains `account number<TAB>balance in cents<TAB>customer name`. Loading validates the complete file before replacing the current list. Saving writes a temporary file, then replaces the previous file only after the write succeeds. A failed save is reported so the user can retry.

History rows follow their account, with transaction type, amount, balance after the operation, and the other account number for transfers. See README.md for the full file format. Older files still load without inventing past transactions.

## Assignment context

This educational model supports opening accounts, updating customer names, inspecting balances and histories, making money operations, and closing zero-balance accounts. It is a local, single-user simulation. Authentication, encryption, interest, and concurrent users are outside its scope. Use fictional customer data for demonstrations.

Binaries, downloaded tools, and saved account data are excluded by `.gitignore`. Submit the source, documentation, VS Code configuration, scripts, and tests. Your teacher's fresh clone starts empty.
