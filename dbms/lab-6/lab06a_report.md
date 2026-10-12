# Lab 06a: Intermediate SQL Practice & Analytics

**Database:** StreamFlix (SQLite)  
**Deliverables:** `queries.sql`, `lab06a_report.md`

---

## Task 1: User Viewing Profile & Binge Engagement

### SQL Mechanics
The query aggregates viewing metrics per user using `SUM()`, `COUNT(DISTINCT)`, and `AVG()` paired with a conditional `CASE` expression for engagement tier classification. The `HAVING` clause filters post-aggregation to retain only users with at least two distinct titles watched before sorting by total watch time descending.

### Output Table
| user_id | name | subscription_tier | total_watch_time_mins | titles_watched_count | avg_session_duration | engagement_category |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 1 | Aarav Sharma | Premium | 460 | 5 | 92.0 | Power Viewer |
| 3 | Liam Chen | Premium | 380 | 4 | 95.0 | Power Viewer |
| 2 | Sophia Martinez | Standard | 250 | 2 | 125.0 | Regular Viewer |
| 5 | Carlos Gomez | Standard | 240 | 3 | 80.0 | Regular Viewer |
| 6 | Yuki Tanaka | Premium | 145 | 2 | 72.5 | Light Viewer |
| 4 | Emma Watson | Basic | 130 | 2 | 65.0 | Light Viewer |

---

## Task 2: Top Genre Rankings per Subscription Tier

### SQL Mechanics
A Common Table Expression (CTE) first groups viewing history by subscription tier and content genre to aggregate total watch duration. The second CTE applies `DENSE_RANK()` partitioned by `subscription_tier` ordered by duration descending, allowing the outer query to filter for the top two genres per tier.

### Output Table
| subscription_tier | genre | total_watch_time | genre_rank |
| :--- | :--- | :--- | :--- |
| Basic | Comedy | 85 | 1 |
| Basic | Drama | 45 | 2 |
| Premium | Sci-Fi | 430 | 1 |
| Premium | Action | 260 | 2 |
| Standard | Drama | 265 | 1 |
| Standard | Sci-Fi | 140 | 2 |

---

##  Task 3: Inactive Active Subscriber Identification

### SQL Mechanics
A correlated `NOT EXISTS` anti-join subquery filters out active subscribers who logged any viewing session in the 30 days preceding 2026-03-01. The outer query aggregates the remaining qualified subscribers to calculate their last watch date and enforces the historical rating threshold (`>= 4.0`) via the `HAVING` clause.

### Output Table
| user_id | name | email | subscription_tier | last_watch_date | avg_rating_given |
| :--- | :--- | :--- | :--- | :--- | :--- |
| 6 | Yuki Tanaka | yuki@streamflix.com | Premium | 2026-01-10 | 4.5 |
| 2 | Sophia Martinez | sophia@streamflix.com | Standard | 2026-01-15 | 4.5 |

---

## Task 4: User Session Continuity & Gap Analysis

### SQL Mechanics
The `LEAD()` window function partitions chronologically sorted sessions by user to retrieve each subsequent session date within the CTE. In the outer query, SQLite `JULIANDAY` arithmetic computes the interval between consecutive sessions, flagging intervals of 14 or more days while filtering out terminal user sessions where `next_watch_date` is `NULL`.

### Output Table
| watch_id | user_id | name | watch_date | next_watch_date | days_until_next_session | session_gap_flag |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 1 | 1 | Aarav Sharma | 2026-01-05 | 2026-01-10 | 5 | NORMAL |
| 2 | 1 | Aarav Sharma | 2026-01-10 | 2026-01-20 | 10 | NORMAL |
| 3 | 1 | Aarav Sharma | 2026-01-20 | 2026-02-15 | 26 | LONG INACTIVITY BREAK |
| 4 | 1 | Aarav Sharma | 2026-02-15 | 2026-02-25 | 10 | NORMAL |
| 6 | 2 | Sophia Martinez | 2026-01-02 | 2026-01-15 | 13 | NORMAL |
| 8 | 3 | Liam Chen | 2026-01-08 | 2026-01-18 | 10 | NORMAL |
| 9 | 3 | Liam Chen | 2026-01-18 | 2026-02-05 | 18 | LONG INACTIVITY BREAK |
| 10 | 3 | Liam Chen | 2026-02-05 | 2026-02-22 | 17 | LONG INACTIVITY BREAK |
| 12 | 4 | Emma Watson | 2026-01-05 | 2026-01-25 | 20 | LONG INACTIVITY BREAK |
| 14 | 5 | Carlos Gomez | 2026-01-12 | 2026-01-28 | 16 | LONG INACTIVITY BREAK |
| 15 | 5 | Carlos Gomez | 2026-01-28 | 2026-02-18 | 21 | LONG INACTIVITY BREAK |
| 17 | 6 | Yuki Tanaka | 2026-01-05 | 2026-01-10 | 5 | NORMAL |

---

## Task 5: Content Catalog Performance & View Creation

### SQL Mechanics
The view connects catalog content released from 2024 onwards to watch and rating metrics using `LEFT JOIN` onto pre-aggregated subqueries, preventing Cartesian product duplication across independent 1-to-many relationships. `COALESCE` safely defaults unwatched titles to zero views and zero watch hours while unrated titles naturally evaluate to `NULL` for average rating.

### Output Table
| content_id | title | genre | release_year | type | total_views | total_watch_hours | avg_user_rating | rating_count |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 101 | Cyber Horizon | Sci-Fi | 2024 | Movie | 3 | 7.0 | 4.67 | 3 |
| 106 | Shadow Operative | Action | 2025 | Movie | 2 | 4.33 | 4.5 | 2 |
| 105 | Deep Ocean Wonders | Documentary | 2024 | Movie | 2 | 3.17 | 4.0 | 1 |
| 104 | Laugh Factory Live | Comedy | 2024 | Movie | 2 | 2.83 | 3.0 | 1 |
| 103 | Quantum Leap 2099 | Sci-Fi | 2025 | Series | 3 | 2.5 | 5.0 | 1 |
| 107 | Silicon Valley Chronicles | Drama | 2024 | Series | 3 | 2.25 | NULL | 0 |
| 108 | Future Earth | Documentary | 2025 | Series | 1 | 1.0 | NULL | 0 |
