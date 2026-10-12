-- Lab 06a: Medium SQL Practice (StreamFlix Database)
-- data.sql: Sample Data Population

-- 1. Users
INSERT INTO users (user_id, name, email, country, signup_date, subscription_tier) VALUES
(1, 'Aarav Sharma', 'aarav@streamflix.com', 'India', '2025-06-01', 'Premium'),
(2, 'Sophia Martinez', 'sophia@streamflix.com', 'USA', '2025-07-15', 'Standard'),
(3, 'Liam Chen', 'liam@streamflix.com', 'Canada', '2025-08-10', 'Premium'),
(4, 'Emma Watson', 'emma@streamflix.com', 'UK', '2025-09-01', 'Basic'),
(5, 'Carlos Gomez', 'carlos@streamflix.com', 'Spain', '2025-10-05', 'Standard'),
(6, 'Yuki Tanaka', 'yuki@streamflix.com', 'Japan', '2025-11-20', 'Premium'),
(7, 'Maya Lin', 'maya@streamflix.com', 'USA', '2025-12-01', 'Basic'),
(8, 'Noah Smith', 'noah@streamflix.com', 'UK', '2026-01-10', 'Standard');

-- 2. Content Catalog
INSERT INTO content (content_id, title, genre, release_year, duration_minutes, type) VALUES
(101, 'Cyber Horizon', 'Sci-Fi', 2024, 140, 'Movie'),
(102, 'The Last Dynasty', 'Drama', 2023, 110, 'Movie'),
(103, 'Quantum Leap 2099', 'Sci-Fi', 2025, 50, 'Series'),
(104, 'Laugh Factory Live', 'Comedy', 2024, 85, 'Movie'),
(105, 'Deep Ocean Wonders', 'Documentary', 2024, 95, 'Movie'),
(106, 'Shadow Operative', 'Action', 2025, 130, 'Movie'),
(107, 'Silicon Valley Chronicles', 'Drama', 2024, 45, 'Series'),
(108, 'Future Earth', 'Documentary', 2025, 60, 'Series');

-- 3. Subscriptions
INSERT INTO subscriptions (sub_id, user_id, start_date, end_date, monthly_amount, payment_status) VALUES
(1, 1, '2025-06-01', NULL, 15.99, 'active'),
(2, 2, '2025-07-15', NULL, 11.99, 'active'),
(3, 3, '2025-08-10', NULL, 15.99, 'active'),
(4, 4, '2025-09-01', '2026-01-31', 7.99, 'expired'),
(5, 5, '2025-10-05', NULL, 11.99, 'active'),
(6, 6, '2025-11-20', NULL, 15.99, 'active'),
(7, 7, '2025-12-01', '2026-02-15', 7.99, 'cancelled'),
(8, 8, '2026-01-10', NULL, 11.99, 'active');

-- 4. Watch History (Dates set around Jan-Feb 2026; Cut-off date for queries is 2026-03-01)
INSERT INTO watch_history (watch_id, user_id, content_id, watch_date, watch_time_minutes, device, completed) VALUES
-- User 1 (Aarav): Heavy viewer
(1, 1, 101, '2026-01-05', 140, 'TV', 1),
(2, 1, 103, '2026-01-10', 50, 'TV', 1),
(3, 1, 106, '2026-01-20', 130, 'TV', 1),
(4, 1, 107, '2026-02-15', 45, 'TV', 1),
(5, 1, 105, '2026-02-25', 95, 'TV', 1), -- Watch time total = 460 mins

-- User 2 (Sophia): Active subscriber, but hasn't watched anything in February (last watch Jan 15)
(6, 2, 101, '2026-01-02', 140, 'Web', 1),
(7, 2, 102, '2026-01-15', 110, 'Web', 1), -- Last watch date: 2026-01-15 (>30 days before March 1)

-- User 3 (Liam): Active viewer
(8, 3, 101, '2026-01-08', 140, 'TV', 1),
(9, 3, 103, '2026-01-18', 50, 'Mobile', 1),
(10, 3, 106, '2026-02-05', 130, 'TV', 1),
(11, 3, 108, '2026-02-22', 60, 'TV', 1),

-- User 4 (Emma): Basic tier, watched in Jan
(12, 4, 104, '2026-01-05', 85, 'Mobile', 1),
(13, 4, 107, '2026-01-25', 45, 'Mobile', 1),

-- User 5 (Carlos): Active subscriber, watched in Feb
(14, 5, 102, '2026-01-12', 110, 'Web', 1),
(15, 5, 104, '2026-01-28', 85, 'Web', 1),
(16, 5, 107, '2026-02-18', 45, 'Web', 1),

-- User 6 (Yuki): Active subscriber, but last watch date was Jan 10 (>30 days before March 1)
(17, 6, 103, '2026-01-05', 50, 'TV', 1),
(18, 6, 105, '2026-01-10', 95, 'TV', 1);

-- 5. Content Ratings
INSERT INTO ratings (rating_id, user_id, content_id, score, rating_date) VALUES
(1, 1, 101, 5, '2026-01-05'),
(2, 1, 106, 4, '2026-01-20'),
(3, 2, 101, 5, '2026-01-02'),
(4, 2, 102, 4, '2026-01-15'), -- Sophia avg rating = 4.5
(5, 3, 101, 4, '2026-01-08'),
(6, 3, 106, 5, '2026-02-05'),
(7, 4, 104, 3, '2026-01-05'),
(8, 5, 102, 4, '2026-01-12'),
(9, 6, 103, 5, '2026-01-05'),
(10, 6, 105, 4, '2026-01-10'); -- Yuki avg rating = 4.5
