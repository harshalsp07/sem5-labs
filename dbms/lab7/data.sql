-- Lab 07: Medium-Advanced SQL Mastery (FinPay Database)
-- data.sql: Sample Data Population

-- 1. Users (with Referral Tree: Aarav [1] -> Liam [2] -> Sophia [3] -> Emma [4], etc.)
INSERT INTO users (user_id, name, email, kyc_status, signup_date, referrer_id) VALUES
(1, 'Aarav Sharma', 'aarav@finpay.com', 'verified', '2025-01-01', NULL),
(2, 'Liam Chen', 'liam@finpay.com', 'verified', '2025-01-15', 1),
(3, 'Sophia Martinez', 'sophia@finpay.com', 'pending', '2025-02-01', 2),
(4, 'Emma Watson', 'emma@finpay.com', 'flagged', '2025-02-15', 3),
(5, 'Carlos Gomez', 'carlos@finpay.com', 'verified', '2025-03-01', 1),
(6, 'Yuki Tanaka', 'yuki@finpay.com', 'pending', '2025-03-10', 5),
(7, 'Noah Smith', 'noah@finpay.com', 'flagged', '2025-03-20', NULL),
(8, 'Diana Prince', 'diana@finpay.com', 'verified', '2025-04-01', NULL);

-- 2. Accounts
INSERT INTO accounts (account_id, user_id, account_type, balance, created_date) VALUES
(101, 1, 'wallet', 12500.00, '2025-01-01'),
(102, 1, 'savings', 45000.00, '2025-01-02'),
(103, 2, 'wallet', 3200.00, '2025-01-15'),
(104, 3, 'wallet', 850.00, '2025-02-01'),
(105, 4, 'wallet', 150.00, '2025-02-15'),  -- Unverified/flagged account
(106, 5, 'wallet', 8900.00, '2025-03-01'),
(107, 6, 'checking', 2100.00, '2025-03-10'), -- Pending KYC account
(108, 7, 'wallet', 5400.00, '2025-03-20'); -- Flagged KYC account

-- 3. Merchants
INSERT INTO merchants (merchant_id, merchant_name, category, commission_rate) VALUES
(201, 'MegaMart Retail', 'Retail', 1.50),
(202, 'Urban Express Supermarket', 'Retail', 1.20),
(203, 'SkyWings Airlines', 'Travel', 2.50),
(204, 'Globe Trotter Travels', 'Travel', 2.00),
(205, 'Bistro Deluxe', 'Food', 1.80),
(206, 'PowerGrid Utilities', 'Utilities', 0.50),
(207, 'CineMax Entertainment', 'Entertainment', 2.20);

-- 4. Transactions (Dates around Jan-Feb 2026)
INSERT INTO transactions (txn_id, sender_account_id, receiver_account_id, merchant_id, txn_date, txn_time, amount, txn_type, status) VALUES
-- Deposits
(1, NULL, 101, NULL, '2026-01-01', '09:00:00', 10000.00, 'deposit', 'success'),
(2, NULL, 103, NULL, '2026-01-02', '10:00:00', 5000.00, 'deposit', 'success'),
(3, NULL, 107, NULL, '2026-01-05', '11:00:00', 3000.00, 'deposit', 'success'),

-- P2P Transfers & Rapid Velocity Test for Account 101 (Aarav): 2 transfers within 3 minutes totaling 2500
(4, 101, 103, NULL, '2026-01-10', '14:00:00', 1200.00, 'p2p', 'success'),
(5, 101, 104, NULL, '2026-01-10', '14:02:30', 1300.00, 'p2p', 'success'), -- Rapid velocity spike! (gap = 150 sec, combined = 2500)

-- Merchant Payments
(6, 101, NULL, 201, '2026-01-12', '16:30:00', 2500.00, 'merchant_payment', 'success'),
(7, 103, NULL, 201, '2026-01-15', '18:15:00', 1800.00, 'merchant_payment', 'success'),
(8, 106, NULL, 203, '2026-01-20', '11:20:00', 6500.00, 'merchant_payment', 'success'),
(9, 103, NULL, 204, '2026-01-22', '09:45:00', 3200.00, 'merchant_payment', 'success'),
(10, 101, NULL, 205, '2026-01-25', '20:00:00', 450.00, 'merchant_payment', 'success'),
(11, 106, NULL, 206, '2026-01-28', '14:10:00', 1200.00, 'merchant_payment', 'success'),

-- Failed / Reversed Transactions (for audit view testing)
(12, 105, 103, NULL, '2026-02-01', '10:15:00', 1500.00, 'p2p', 'failed'),   -- Account 105 (Emma, flagged KYC)
(13, 105, 104, NULL, '2026-02-01', '10:18:00', 2000.00, 'p2p', 'failed'),   -- Account 105 2nd failed transaction!
(14, 107, NULL, 203, '2026-02-05', '13:00:00', 5000.00, 'merchant_payment', 'reversed'), -- Account 107 (Yuki, pending KYC)
(15, 108, 101, NULL, '2026-02-10', '15:45:00', 4000.00, 'p2p', 'failed');   -- Account 108 (Noah, flagged KYC)

-- 5. Risk Alerts
INSERT INTO risk_alerts (alert_id, txn_id, risk_score, flag_reason, alert_date) VALUES
(1, 5, 88, 'Rapid consecutive high-value P2P transfer', '2026-01-10'),
(2, 12, 92, 'Unverified account transfer attempt', '2026-02-01'),
(3, 14, 85, 'Reversed travel payment exceeding threshold', '2026-02-05'),
(4, 15, 95, 'Suspicious account activity flag', '2026-02-10');
