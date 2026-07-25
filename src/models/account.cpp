#include "models/account.h"
#include <cstdio>

namespace {
constexpr double kDefaultStartingBalance = 500.0;
}

static Account rowToAccount(const DB::Row& r) {
    Account a;
    a.id             = std::stoi(r.at("id"));
    a.user_id        = std::stoi(r.at("user_id"));
    a.account_number = r.at("account_number");
    a.balance        = std::stod(r.at("balance"));
    a.created_at     = r.at("created_at");
    return a;
}

static std::string generateAccountNumber(int user_id) {
    char buf[12];
    std::snprintf(buf, sizeof(buf), "ACC%06d", user_id);
    return std::string(buf);
}

namespace AccountModel {

Account create(DB& db, int user_id) {
    std::string acc_num = generateAccountNumber(user_id);
    db.execute(
        "INSERT INTO accounts (user_id, account_number, balance) VALUES (?, ?, ?);",
        [&](sqlite3_stmt* s) {
            sqlite3_bind_int (s, 1, user_id);
            sqlite3_bind_text(s, 2, acc_num.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(s, 3, kDefaultStartingBalance);
        }
    );
    return *findByUserId(db, user_id);
}

std::optional<Account> findByUserId(DB& db, int user_id) {
    auto rows = db.query(
        "SELECT * FROM accounts WHERE user_id = ? LIMIT 1;",
        [user_id](sqlite3_stmt* s) { sqlite3_bind_int(s, 1, user_id); }
    );
    if (rows.empty()) return std::nullopt;
    return rowToAccount(rows[0]);
}

std::optional<Account> findByAccountNumber(DB& db, const std::string& account_number) {
    auto rows = db.query(
        "SELECT * FROM accounts WHERE account_number = ? LIMIT 1;",
        [&](sqlite3_stmt* s) {
            sqlite3_bind_text(s, 1, account_number.c_str(), -1, SQLITE_TRANSIENT);
        }
    );
    if (rows.empty()) return std::nullopt;
    return rowToAccount(rows[0]);
}

bool updateBalance(DB& db, int account_id, double new_balance) {
    db.execute(
        "UPDATE accounts SET balance = ? WHERE id = ?;",
        [&](sqlite3_stmt* s) {
            sqlite3_bind_double(s, 1, new_balance);
            sqlite3_bind_int   (s, 2, account_id);
        }
    );
    return true;
}

std::vector<Account> findAll(DB& db) {
    auto rows = db.query("SELECT * FROM accounts ORDER BY id;");
    std::vector<Account> accounts;
    accounts.reserve(rows.size());
    for (const auto& r : rows) accounts.push_back(rowToAccount(r));
    return accounts;
}

} // namespace AccountModel
