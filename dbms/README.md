# Library Lab 4

This project is a small library catalog with SQLite support by default and PostgreSQL support via the same app logic.

## Part 2.1 — Transactions and atomicity

The two writes that must happen together are:
- decrementing the book’s copies count in the books table, and
- inserting a new row into the loans table.

If only one of these happens, the database becomes inconsistent: the library would show a book as borrowed or returned without a matching loan record, or a loan without the stock being updated. That breaks the rules of the catalog and can leave copies and loans out of sync.

The rollback occurs in the exception handler inside borrow_book(). When a book has zero copies, the code raises a ValueError, the except block runs, and conn.rollback() restores the database to the state it had before the transaction began.

## Part 2.2 — Atomicity proof

Borrowing book 4 with zero copies fails and leaves the database unchanged:
- the operation returns False,
- no new loan row is created,
- the copies value for book 4 remains 0.

This is exactly what the rollback is meant to guarantee.

## Part 2.3 — Bug injection and rollback demo

Before the deliberate bug, the copies value for the affected book was 0. After the injected RuntimeError("boom") was placed between the UPDATE and INSERT, the database was still left unchanged because the transaction rolled back. In other words, the value stayed at 0 both before and after the failed attempt.

## Part 3.2 — Why the SQL injection works

The vulnerable query built SQL by string concatenation, so the user input was pasted directly into the SQL text. Once the input contained SQL syntax like `% ' OR '1'='1' --`, it changed the meaning of the WHERE clause and allowed extra rows to be returned.

## Part 3.3 — Safe fix

The application now uses the safe parameterized query through search_books() instead of string concatenation. The attack strings are treated as literal text and return no rows, while normal searches such as Database still work.

Rule: Never build SQL by string concatenation; always pass user input through placeholders.

## Part 4.3 — PostgreSQL migration changes

The changes needed to move from SQLite to PostgreSQL were small:
- the placeholder changed from ? to %s,
- the connection string in connect() was updated for the local PostgreSQL setup,
- the app still uses the same SQL logic because the parameterized approach stays consistent across both engines.

## Basic project result

The app supports SQLite by default and can be switched to PostgreSQL with the --backend postgres flag.
