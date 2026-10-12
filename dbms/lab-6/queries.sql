
-- Part 1 — Task 1: User Viewing Profile & Binge Engagement (Aggregations & Grouping)
SELECT 
    u.user_id,
    u.name,
    u.subscription_tier,
    SUM(w.watch_time_minutes) AS total_watch_time_mins,
    COUNT(DISTINCT w.content_id) AS titles_watched_count,
    ROUND(AVG(w.watch_time_minutes), 1) AS avg_session_duration,
    CASE 
        WHEN SUM(w.watch_time_minutes) >= 350 THEN 'Power Viewer'
        WHEN SUM(w.watch_time_minutes) >= 150 THEN 'Regular Viewer'
        ELSE 'Light Viewer'
    END AS engagement_category
FROM users u
JOIN watch_history w ON u.user_id = w.user_id
GROUP BY u.user_id, u.name, u.subscription_tier
HAVING COUNT(DISTINCT w.content_id) >= 2
ORDER BY total_watch_time_mins DESC;


-- Part 2 — Task 2: Top Genre Rankings per Subscription Tier (Window Ranking)
WITH genre_watch AS (
    SELECT 
        u.subscription_tier,
        c.genre,
        SUM(w.watch_time_minutes) AS total_watch_time
    FROM users u
    JOIN watch_history w ON u.user_id = w.user_id
    JOIN content c ON w.content_id = c.content_id
    GROUP BY u.subscription_tier, c.genre
),
ranked_genres AS (
    SELECT 
        subscription_tier,
        genre,
        total_watch_time,
        DENSE_RANK() OVER (
            PARTITION BY subscription_tier 
            ORDER BY total_watch_time DESC
        ) AS genre_rank
    FROM genre_watch
)
SELECT 
    subscription_tier,
    genre,
    total_watch_time,
    genre_rank
FROM ranked_genres
WHERE genre_rank <= 2
ORDER BY subscription_tier ASC, genre_rank ASC;


-- Part 3 — Task 3: Inactive Active Subscriber Identification (Correlated Subqueries & Anti-Joins)
SELECT 
    u.user_id,
    u.name,
    u.email,
    u.subscription_tier,
    MAX(w.watch_date) AS last_watch_date,
    ROUND(AVG(r.score), 2) AS avg_rating_given
FROM users u
JOIN subscriptions s ON u.user_id = s.user_id
JOIN watch_history w ON u.user_id = w.user_id
JOIN ratings r ON u.user_id = r.user_id
WHERE s.payment_status = 'active'
  AND NOT EXISTS (
      SELECT 1 
      FROM watch_history wh 
      WHERE wh.user_id = u.user_id 
        AND wh.watch_date >= '2026-01-30'
  )
GROUP BY u.user_id, u.name, u.email, u.subscription_tier
HAVING AVG(r.score) >= 4.0
ORDER BY last_watch_date ASC;


-- Part 4 — Task 4: User Session Continuity & Gap Analysis (Lead Window Functions)
WITH session_lead AS (
    SELECT 
        w.watch_id,
        w.user_id,
        u.name,
        w.watch_date,
        LEAD(w.watch_date) OVER (
            PARTITION BY w.user_id 
            ORDER BY w.watch_date
        ) AS next_watch_date
    FROM watch_history w
    JOIN users u ON w.user_id = u.user_id
)
SELECT 
    watch_id,
    user_id,
    name,
    watch_date,
    next_watch_date,
    CAST(JULIANDAY(next_watch_date) - JULIANDAY(watch_date) AS INTEGER) AS days_until_next_session,
    CASE 
        WHEN (JULIANDAY(next_watch_date) - JULIANDAY(watch_date)) >= 14 THEN 'LONG INACTIVITY BREAK'
        ELSE 'NORMAL'
    END AS session_gap_flag
FROM session_lead
WHERE next_watch_date IS NOT NULL
ORDER BY user_id ASC, watch_date ASC;


-- Part 5 — Task 5: Content Catalog Performance & View Creation (Database Views & Outer Joins)
DROP VIEW IF EXISTS v_content_analytics;

CREATE VIEW v_content_analytics AS
SELECT 
    c.content_id,
    c.title,
    c.genre,
    c.release_year,
    c.type,
    COALESCE(w.total_views, 0) AS total_views,
    COALESCE(w.total_watch_hours, 0.0) AS total_watch_hours,
    r.avg_user_rating,
    COALESCE(r.rating_count, 0) AS rating_count
FROM content c
LEFT JOIN (
    SELECT 
        content_id,
        COUNT(watch_id) AS total_views,
        ROUND(SUM(watch_time_minutes) / 60.0, 2) AS total_watch_hours
    FROM watch_history
    GROUP BY content_id
) w ON c.content_id = w.content_id
LEFT JOIN (
    SELECT 
        content_id,
        ROUND(AVG(score), 2) AS avg_user_rating,
        COUNT(rating_id) AS rating_count
    FROM ratings
    GROUP BY content_id
) r ON c.content_id = r.content_id
WHERE c.release_year >= 2024;

-- Verification Query:
SELECT * FROM v_content_analytics ORDER BY total_watch_hours DESC;
