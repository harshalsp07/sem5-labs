-- Lab 06a: Medium SQL Practice (StreamFlix Database)
-- schema.sql: Relational Schema DDL (PostgreSQL & SQLite Portable)

DROP TABLE IF EXISTS ratings;
DROP TABLE IF EXISTS watch_history;
DROP TABLE IF EXISTS subscriptions;
DROP TABLE IF EXISTS content;
DROP TABLE IF EXISTS users;

-- 1. Users Table
CREATE TABLE users (
    user_id INTEGER PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    country VARCHAR(50) NOT NULL,
    signup_date DATE NOT NULL,
    subscription_tier VARCHAR(20) NOT NULL CHECK (subscription_tier IN ('Basic', 'Standard', 'Premium'))
);

-- 2. Content Catalog Table
CREATE TABLE content (
    content_id INTEGER PRIMARY KEY,
    title VARCHAR(150) NOT NULL,
    genre VARCHAR(50) NOT NULL,
    release_year INTEGER NOT NULL,
    duration_minutes INTEGER NOT NULL CHECK (duration_minutes > 0),
    type VARCHAR(20) NOT NULL CHECK (type IN ('Movie', 'Series'))
);

-- 3. Subscriptions Table
CREATE TABLE subscriptions (
    sub_id INTEGER PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    start_date DATE NOT NULL,
    end_date DATE,
    monthly_amount NUMERIC(10, 2) NOT NULL CHECK (monthly_amount >= 0),
    payment_status VARCHAR(20) NOT NULL CHECK (payment_status IN ('active', 'cancelled', 'expired'))
);

-- 4. Watch History Table
CREATE TABLE watch_history (
    watch_id INTEGER PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    content_id INTEGER NOT NULL REFERENCES content(content_id) ON DELETE CASCADE,
    watch_date DATE NOT NULL,
    watch_time_minutes INTEGER NOT NULL CHECK (watch_time_minutes > 0),
    device VARCHAR(20) NOT NULL CHECK (device IN ('Mobile', 'TV', 'Web')),
    completed INTEGER NOT NULL CHECK (completed IN (0, 1)) -- 1 = completed, 0 = partial
);

-- 5. Content Ratings Table
CREATE TABLE ratings (
    rating_id INTEGER PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    content_id INTEGER NOT NULL REFERENCES content(content_id) ON DELETE CASCADE,
    score INTEGER NOT NULL CHECK (score BETWEEN 1 AND 5),
    rating_date DATE NOT NULL,
    UNIQUE (user_id, content_id)
);
