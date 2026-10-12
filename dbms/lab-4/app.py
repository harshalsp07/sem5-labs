#!/usr/bin/env python3
"""
Lab 4 starter — a menu-driven Library catalog CLI backed by a real database.

This scaffold runs on SQLite out of the box (no setup). Part of the lab is to
(a) understand it, (b) complete the TODOs, (c) demonstrate + fix an SQL-injection
bug, and (d) repoint it to PostgreSQL.

Usage:
    python app.py                      # interactive menu (SQLite: library.db)
    python app.py --db /tmp/lib.db     # choose the database file
    python app.py --backend postgres   # use PostgreSQL (see connect())
    python app.py --selftest           # run automated checks (used for grading)

Key ideas demonstrated:
    * DB-API workflow: connect -> cursor -> execute -> commit/rollback -> close
    * PARAMETERIZED queries (the `?` / `%s` placeholders) -> injection-safe
    * a multi-statement TRANSACTION with COMMIT on success / ROLLBACK on failure
"""
import argparse
import sqlite3
import sys

# ---------------------------------------------------------------------------
# Backend selection
# ---------------------------------------------------------------------------
# SQLite uses "?" placeholders; psycopg (PostgreSQL) uses "%s". We keep a single
# PH string so the same SQL text works on both backends.
BACKEND = "sqlite"
PH = "?"


def connect(args):
    """Return a live DB connection for the chosen backend."""
    global BACKEND, PH
    BACKEND = args.backend
    if args.backend == "sqlite":
        PH = "?"
        conn = sqlite3.connect(args.db)
        conn.execute("PRAGMA foreign_keys = ON;")   # SQLite: enforce FKs
        return conn
    elif args.backend == "postgres":
        PH = "%s"
        try:
            import psycopg
        except ImportError:
            sys.exit("psycopg not installed. Run: pip install 'psycopg[binary]'")
        # TODO (Part 4): adjust the connection string for your PostgreSQL setup.
        return psycopg.connect(
            "host=localhost dbname=library user=csl303 password=csl303"
        )
    else:
        sys.exit(f"unknown backend: {args.backend}")


# ---------------------------------------------------------------------------
# Schema + seed data
# ---------------------------------------------------------------------------
def schema_sql():
    loan_id_type = "SERIAL" if BACKEND == "postgres" else "INTEGER"
    return [
        """CREATE TABLE IF NOT EXISTS books (
               book_id   INTEGER PRIMARY KEY,
               title     TEXT NOT NULL,
               author    TEXT NOT NULL,
               copies    INTEGER NOT NULL CHECK (copies >= 0)
           )""",
        """CREATE TABLE IF NOT EXISTS members (
               member_id INTEGER PRIMARY KEY,
               name      TEXT NOT NULL
           )""",
        f"""CREATE TABLE IF NOT EXISTS loans (
               loan_id   {loan_id_type} PRIMARY KEY,
               book_id   INTEGER NOT NULL REFERENCES books(book_id),
               member_id INTEGER NOT NULL REFERENCES members(member_id),
               returned  INTEGER NOT NULL DEFAULT 0
           )""",
    ]

SEED_BOOKS = [
    (1, "Database System Concepts", "Silberschatz", 3),
    (2, "Database Internals",       "Petrov",       2),
    (3, "The Pragmatic Programmer", "Hunt & Thomas", 1),
    (4, "SQL Performance Explained", "Winand",       0),   # none available
]
SEED_MEMBERS = [(1, "Aarav"), (2, "Diya"), (3, "Kabir")]


def init_db(conn):
    """Create tables and seed data if the DB is empty."""
    cur = conn.cursor()
    for stmt in schema_sql():
        cur.execute(stmt)
    cur.execute("SELECT COUNT(*) FROM books")
    if cur.fetchone()[0] == 0:
        cur.executemany(
            f"INSERT INTO books (book_id, title, author, copies) "
            f"VALUES ({PH}, {PH}, {PH}, {PH})", SEED_BOOKS)
        cur.executemany(
            f"INSERT INTO members (member_id, name) VALUES ({PH}, {PH})",
            SEED_MEMBERS)
    conn.commit()


# ---------------------------------------------------------------------------
# Operations  (all use PARAMETERIZED queries)
# ---------------------------------------------------------------------------
def list_books(conn):
    cur = conn.cursor()
    cur.execute("SELECT book_id, title, author, copies FROM books ORDER BY book_id")
    print("\n  id | title                         | author         | copies")
    print("  ---+-------------------------------+----------------+-------")
    for bid, title, author, copies in cur.fetchall():
        print(f"  {bid:>2} | {title:<29} | {author:<14} | {copies}")
    print()


def add_book(conn, book_id, title, author, copies):
    """Parameterized INSERT — user data is never concatenated into SQL."""
    conn.execute(
        f"INSERT INTO books (book_id, title, author, copies) "
        f"VALUES ({PH}, {PH}, {PH}, {PH})",
        (book_id, title, author, copies))
    conn.commit()


def search_books(conn, term):
    """SAFE search: the search term is passed as a parameter."""
    cur = conn.cursor()
    cur.execute(
        f"SELECT book_id, title, author FROM books WHERE title LIKE {PH}",
        (f"%{term}%",))
    return cur.fetchall()


# Intentionally vulnerable: kept only to demonstrate SQL injection in the lab.
# The menu no longer calls this function; we use the safe parameterized search.
def search_books_unsafe(conn, term):
    """UNSAFE search (for the injection demo ONLY — never do this).

    Builds SQL by string concatenation, so a crafted `term` becomes part of the
    command. Part 4 asks you to demonstrate an attack against this and then
    switch the app to use search_books() above.
    """
    sql = f"SELECT book_id, title, author FROM books WHERE title LIKE '%{term}%'"
    cur = conn.cursor()
    cur.executescript(sql) if BACKEND == "sqlite" and ";" in term else cur.execute(sql)
    return cur.fetchall()


def list_loans(conn):
    """Print all currently outstanding loans with member and book names."""
    cur = conn.cursor()
    cur.execute(
        "SELECT m.name, b.title FROM loans l "
        "JOIN members m ON m.member_id = l.member_id "
        "JOIN books b ON b.book_id = l.book_id "
        "WHERE l.returned = 0 ORDER BY l.loan_id"
    )
    rows = cur.fetchall()
    if not rows:
        print("  No outstanding loans.")
        return
    print("\n  member        | book")
    print("  -------------+----------------------------------")
    for member_name, title in rows:
        print(f"  {member_name:<12} | {title}")
    print()


def borrow_book(conn, book_id, member_id):
    """A TRANSACTION: decrement copies AND record a loan, atomically.

    If no copies are available we raise, and the whole transaction is rolled
    back — so we never record a loan without also decrementing a copy.
    """
    cur = conn.cursor()
    try:
        # (SQLite autocommits single statements; we open an explicit transaction)
        if BACKEND == "sqlite":
            conn.execute("BEGIN")
        cur.execute(f"SELECT copies FROM books WHERE book_id = {PH}", (book_id,))
        row = cur.fetchone()
        if row is None:
            raise ValueError(f"no such book: {book_id}")
        if row[0] <= 0:
            raise ValueError("no copies available")
        cur.execute(f"UPDATE books SET copies = copies - 1 WHERE book_id = {PH}",
                    (book_id,))
        cur.execute(
            f"INSERT INTO loans (book_id, member_id) VALUES ({PH}, {PH})",
            (book_id, member_id))
        conn.commit()
        return True
    except Exception as e:
        conn.rollback()
        print("  borrow failed (rolled back):", e)
        return False


def return_book(conn, book_id, member_id):
    """Mark an outstanding loan returned and put one copy back."""
    cur = conn.cursor()
    try:
        if BACKEND == "sqlite":
            conn.execute("BEGIN")
        cur.execute(
            f"SELECT loan_id FROM loans WHERE book_id = {PH} AND member_id = {PH} AND returned = 0",
            (book_id, member_id),
        )
        row = cur.fetchone()
        if row is None:
            raise ValueError(f"no outstanding loan for book {book_id} and member {member_id}")

        loan_id = row[0]
        cur.execute(f"UPDATE loans SET returned = 1 WHERE loan_id = {PH}", (loan_id,))
        cur.execute(f"UPDATE books SET copies = copies + 1 WHERE book_id = {PH}", (book_id,))
        conn.commit()
        return True
    except Exception as e:
        conn.rollback()
        print("  return failed (rolled back):", e)
        return False


# ---------------------------------------------------------------------------
# Interactive menu
# ---------------------------------------------------------------------------
MENU = """
==== Library ====
1) List books
2) Add a book
3) Search books (safe)
4) Borrow a book
5) Return a book
6) Search books
7) List outstanding loans
0) Quit
> """


def interactive(conn):
    while True:
        choice = input(MENU).strip()
        if choice == "1":
            list_books(conn)
        elif choice == "2":
            bid = int(input("book_id: "))
            title = input("title: ")
            author = input("author: ")
            copies = int(input("copies: "))
            add_book(conn, bid, title, author, copies)
            print("  added.")
        elif choice == "3":
            term = input("search term: ")
            for r in search_books(conn, term):
                print("  ", r)
        elif choice == "4":
            bid = int(input("book_id: "))
            mid = int(input("member_id: "))
            if borrow_book(conn, bid, mid):
                print("  borrowed.")
        elif choice == "5":
            bid = int(input("book_id: "))
            mid = int(input("member_id: "))
            if return_book(conn, bid, mid):
                print("  returned.")
        elif choice == "6":
            term = input("search term: ")
            for r in search_books(conn, term):
                print("  ", r)
        elif choice == "7":
            list_loans(conn)
        elif choice == "0":
            break
        else:
            print("  ?")
    conn.close()


# ---------------------------------------------------------------------------
# Self-test (deterministic, used for grading / verification)
# ---------------------------------------------------------------------------
def selftest(conn):
    ok = True

    def check(label, cond):
        nonlocal ok
        ok = ok and cond
        print(f"  [{'PASS' if cond else 'FAIL'}] {label}")

    # CRUD
    add_book(conn, 99, "Test Driven Data", "Nobody", 5)
    check("add_book inserts a row",
          conn.execute("SELECT copies FROM books WHERE book_id=99")
              .fetchone()[0] == 5)

    # parameterized search finds the row
    check("safe search finds inserted book",
          any(b[0] == 99 for b in search_books(conn, "Driven")))

    # injection attempt via the SAFE path returns NOTHING (treated as literal)
    inj = "x' OR '1'='1"
    check("safe search is injection-proof (no rows for attack string)",
          len(search_books(conn, inj)) == 0)

    # transaction: borrowing an available book succeeds and decrements copies
    before = conn.execute("SELECT copies FROM books WHERE book_id=1").fetchone()[0]
    borrow_book(conn, 1, 1)
    after = conn.execute("SELECT copies FROM books WHERE book_id=1").fetchone()[0]
    check("borrow decrements copies by 1", after == before - 1)
    check("borrow records exactly one loan",
          conn.execute("SELECT COUNT(*) FROM loans WHERE book_id=1").fetchone()[0] == 1)

    # transaction rollback: borrowing an out-of-stock book (book 4, copies=0)
    loans_before = conn.execute("SELECT COUNT(*) FROM loans").fetchone()[0]
    result = borrow_book(conn, 4, 2)          # should fail + rollback
    loans_after = conn.execute("SELECT COUNT(*) FROM loans").fetchone()[0]
    check("borrow of out-of-stock book fails", result is False)
    check("failed borrow adds NO loan (rolled back)", loans_after == loans_before)
    check("out-of-stock copies unchanged (still 0)",
          conn.execute("SELECT copies FROM books WHERE book_id=4").fetchone()[0] == 0)

    print("\n  RESULT:", "ALL PASS" if ok else "SOME FAILED")
    conn.close()
    return ok


# ---------------------------------------------------------------------------
def main():
    p = argparse.ArgumentParser(description="Lab 4 Library CLI")
    p.add_argument("--db", default="library.db", help="SQLite file (sqlite backend)")
    p.add_argument("--backend", choices=["sqlite", "postgres"], default="sqlite")
    p.add_argument("--selftest", action="store_true", help="run automated checks")
    args = p.parse_args()

    conn = connect(args)
    init_db(conn)

    if args.selftest:
        sys.exit(0 if selftest(conn) else 1)
    interactive(conn)


if __name__ == "__main__":
    main()
