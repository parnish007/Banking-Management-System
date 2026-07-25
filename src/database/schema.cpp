#include "database/schema.h"
#include "auth/password.h"

namespace Schema {

void init(DB& db) {
    db.execute(R"(
        CREATE TABLE IF NOT EXISTS users (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            name          TEXT    NOT NULL,
            username      TEXT    NOT NULL UNIQUE,
            password_hash TEXT    NOT NULL,
            role          TEXT    NOT NULL CHECK(role IN ('user', 'admin')),
            created_at    TEXT    NOT NULL DEFAULT (datetime('now'))
        );
    )");

    db.execute(R"(
        CREATE TABLE IF NOT EXISTS accounts (
            id             INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id        INTEGER NOT NULL REFERENCES users(id),
            account_number TEXT    NOT NULL UNIQUE,
            balance        REAL    NOT NULL DEFAULT 500.0,
            created_at     TEXT    NOT NULL DEFAULT (datetime('now'))
        );
    )");

    db.execute("UPDATE accounts SET balance = 500.0 WHERE balance = 0.0;");

    db.execute(R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            from_account_id INTEGER NOT NULL REFERENCES accounts(id),
            to_account_id   INTEGER NOT NULL REFERENCES accounts(id),
            amount          REAL    NOT NULL CHECK(amount > 0),
            status          TEXT    NOT NULL DEFAULT 'pending'
                                    CHECK(status IN ('pending', 'approved', 'rejected')),
            note            TEXT,
            created_at      TEXT    NOT NULL DEFAULT (datetime('now')),
            reviewed_by     INTEGER REFERENCES users(id),
            reviewed_at     TEXT
        );
    )");

    // Seed default admin only if none exists
    auto rows = db.query("SELECT id FROM users WHERE role = 'admin' LIMIT 1;");
    if (rows.empty()) {
        std::string hash = Password::hash("admin123");
        db.execute(
            "INSERT INTO users (name, username, password_hash, role) VALUES (?, ?, ?, ?);",
            [&](sqlite3_stmt* s) {
                sqlite3_bind_text(s, 1, "Administrator", -1, SQLITE_STATIC);
                sqlite3_bind_text(s, 2, "admin",         -1, SQLITE_STATIC);
                sqlite3_bind_text(s, 3, hash.c_str(),    -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(s, 4, "admin",         -1, SQLITE_STATIC);
            }
        );
    }
}

} // namespace Schema
