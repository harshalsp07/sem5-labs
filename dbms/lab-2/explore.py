#!/usr/bin/env python3
import sqlite3
import sys

conn = sqlite3.connect("lab2.db")
conn.row_factory = sqlite3.Row  # access columns by name
cur = conn.cursor()

# Accept department name from command line
if len(sys.argv) < 2:
    print("Usage: python3 explore.py <department_name>")
    sys.exit(1)

dept = sys.argv[1]

# A parameterized query — the ? is filled safely by the driver
cur.execute("SELECT name, salary FROM instructor WHERE dept_name = ?", (dept,))
print(f"Instructors in {dept}:")
for row in cur.fetchall():
    print(f"  {row['name']}: ${row['salary']}")

# An aggregate
cur.execute("SELECT COUNT(*) AS n FROM student")
print(f"\nTotal students: {cur.fetchone()['n']}")

conn.close()
