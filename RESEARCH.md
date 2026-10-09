# Research: Basic Bank Management System

## Purpose

A Bank Management System organizes customer account information and supports operations on account balances. This assignment focuses on understanding those basic operations and implementing a small console simulation using the C programming language and a linked-list data structure.

## How basic bank accounts work

A checking account allows a customer to deposit money and use it through withdrawals or payments. The Consumer Financial Protection Bureau describes this relationship between funds placed into an account and subsequent spending or withdrawal. [Source: CFPB, checking account questions](https://www.consumerfinance.gov/archive/blog/your-checking-accounts-questions-answered/)

For this project, the account record contains an account number, customer name, and current balance. Deposits increase that balance; withdrawals decrease it. A transfer decreases one account's balance and increases another by the same amount. Account lookup lets the user identify the correct record before changing it.

An overdraft occurs when spending or withdrawals exceed the money available in an account. [Source: CFPB, checking account fees guide](https://files.consumerfinance.gov/f/documents/cfpb_adult-fin-ed_keep-a-lid-on-checking-account-fees.pdf) This simulation rejects overdrafts and requires a zero balance before an account can be deleted. These are project rules chosen to keep account balances nonnegative and prevent deletion of remaining funds.

Real bank systems also address authorization, transaction records, availability of deposited funds, multiple users, and other operational requirements. The assignment model restricts its scope to account records and simple balance changes rather than reproducing a complete bank platform.

## Why use a linked list?

A singly linked list consists of nodes containing data and a pointer to the next node. C structures can hold both a value and a pointer to another structure of the same type, as illustrated by the GNU C Language Manual. [Source: GNU C Language Manual, structures](https://www.gnu.org/software/c-intro-and-ref/manual/c-intro-and-ref.html)

For this assignment, each node holds one account. A pointer identifies the first node, and a null pointer marks the end. The number of accounts can grow by allocating nodes as needed, without selecting a fixed array capacity. Removing an account changes a link and releases the node's memory. Searching still requires traversal, so a lookup takes O(n) time in the worst case. The GNU Astronomy Utilities documentation describes the same node-and-pointer organization and null-terminated chain. [Source: GNU, linked lists](https://www.gnu.org/software/gnuastro/manual/html_node/Linked-lists.html)

The linked list is suitable for demonstrating dynamic allocation, traversal, insertion, and deletion in a small educational program. For a larger application, a database and indexed lookup would be more appropriate design choices.

## Design derived from the research

The implementation provides account creation, listing, search, deposits, withdrawals, transfers, deletion, customer name updates, and transaction history viewing. It checks unique positive account numbers and validates transaction amounts before changing balances. Money is stored in integer cents so decimal amounts do not depend on floating-point rounding. Each account owns a linked list of successful transaction records, including the amount, resulting balance, and the other account number for transfers. Saving accounts and histories to a text file allows the program to restore its state on the next run.

The C program is divided into menu, linked-list, input, and storage modules. This makes each responsibility easier to understand and follows the assignment's requirement to separate code files as needed. [Source: GNU C Reference Manual, program structure](https://www.gnu.org/software/gnu-c-manual/gnu-c-manual.html)

The detailed algorithms, source-file responsibilities, complexity analysis, and compilation instructions are provided in [README.md](README.md).
