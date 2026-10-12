-- Lab 07: Medium-Advanced SQL Mastery (FinPay Database)
-- schema.sql: Relational Schema DDL (PostgreSQL & SQLite Portable)

DROP TABLE IF EXISTS risk_alerts;
DROP TABLE IF EXISTS transactions;
DROP TABLE IF EXISTS merchants;
DROP TABLE IF EXISTS accounts;
DROP TABLE IF EXISTS users;

-- 1. Users Table (with self-referencing referral key)
CREATE TABLE users (
    user_id INTEGER PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    kyc_status VARCHAR(20) NOT NULL CHECK (kyc_status IN ('verified', 'pending', 'flagged')),
    signup_date DATE NOT NULL,
    referrer_id INTEGER REFERENCES users(user_id)
);

-- 2. Accounts Table
CREATE TABLE accounts (
    account_id INTEGER PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    account_type VARCHAR(20) NOT NULL CHECK (account_type IN ('savings', 'checking', 'wallet')),
    balance NUMERIC(12, 2) NOT NULL DEFAULT 0.00,
    created_date DATE NOT NULL
);

-- 3. Merchants Table
CREATE TABLE merchants (
    merchant_id INTEGER PRIMARY KEY,
    merchant_name VARCHAR(100) NOT NULL,
    category VARCHAR(50) NOT NULL CHECK (category IN ('Retail', 'Travel', 'Food', 'Utilities', 'Entertainment')),
    commission_rate NUMERIC(4, 2) NOT NULL CHECK (commission_rate >= 0.0)
);

-- 4. Transactions Table
CREATE TABLE transactions (
    txn_id INTEGER PRIMARY KEY,
    sender_account_id INTEGER REFERENCES accounts(account_id),
    receiver_account_id INTEGER REFERENCES accounts(account_id),
    merchant_id INTEGER REFERENCES merchants(merchant_id),
    txn_date DATE NOT NULL,
    txn_time TIME NOT NULL,
    amount NUMERIC(12, 2) NOT NULL CHECK (amount > 0),
    txn_type VARCHAR(20) NOT NULL CHECK (txn_type IN ('p2p', 'merchant_payment', 'deposit', 'withdrawal')),
    status VARCHAR(20) NOT NULL CHECK (status IN ('success', 'failed', 'reversed'))
);

-- 5. Risk Alerts Table
CREATE TABLE risk_alerts (
    alert_id INTEGER PRIMARY KEY,
    txn_id INTEGER NOT NULL REFERENCES transactions(txn_id) ON DELETE CASCADE,
    risk_score INTEGER NOT NULL CHECK (risk_score BETWEEN 1 AND 100),
    flag_reason VARCHAR(150) NOT NULL,
    alert_date DATE NOT NULL
);
