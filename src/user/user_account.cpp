#include "user/user_account.h"

#include "auth/session.h"
#include "models/account.h"
#include "models/transaction.h"

#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string formatMoney(double amount) {
    std::ostringstream out;
    out << "$" << std::fixed << std::setprecision(2) << amount;
    return out.str();
}

std::string column(const std::string& value, std::size_t width) {
    if (value.size() >= width) {
        return value.substr(0, width);
    }
    return value + std::string(width - value.size(), ' ');
}

std::optional<Account> findAccountById(DB& db, int account_id) {
    const auto rows = db.query(
        "SELECT * FROM accounts WHERE id = ? LIMIT 1;",
        [account_id](sqlite3_stmt* stmt) {
            sqlite3_bind_int(stmt, 1, account_id);
        }
    );

    if (rows.empty()) {
        return std::nullopt;
    }

    const DB::Row& row = rows[0];
    Account account;
    account.id = std::stoi(row.at("id"));
    account.user_id = std::stoi(row.at("user_id"));
    account.account_number = row.at("account_number");
    account.balance = std::stod(row.at("balance"));
    account.created_at = row.at("created_at");
    return account;
}

std::string accountLabel(DB& db, int account_id, int current_account_id) {
    auto account = findAccountById(db, account_id);
    std::string label = account ? account->account_number : "UNKNOWN";
    if (account_id == current_account_id) {
        label += " (you)";
    }
    return label;
}

} // namespace

void UserAccount::showBalanceAndHistory(DB& db) {
    const SessionData session = Session::current();

    auto account = AccountModel::findByUserId(db, session.user_id);
    if (!account) {
        std::cout << "Error: could not load your account. Please contact an admin.\n";
        return;
    }

    std::cout << "\nAccount   : " << account->account_number << "\n";
    std::cout << "Balance   : " << formatMoney(account->balance) << "\n\n";

    const std::vector<Transaction> transactions =
        TransactionModel::findByAccountId(db, account->id);

    if (transactions.empty()) {
        std::cout << "No transactions yet.\n\n";
        return;
    }

    std::cout << column("ID", 5)
              << column("Date", 21)
              << column("From", 18)
              << column("To", 18)
              << column("Amount", 12)
              << column("Status", 12)
              << "Note\n";
    std::cout << std::string(96, '-') << "\n";

    for (const Transaction& txn : transactions) {
        const std::string from = accountLabel(db, txn.from_account_id, account->id);
        const std::string to = accountLabel(db, txn.to_account_id, account->id);
        const std::string note = txn.note.empty() ? "-" : txn.note;

        std::cout << column(std::to_string(txn.id), 5)
                  << column(txn.created_at, 21)
                  << column(from, 18)
                  << column(to, 18)
                  << column(formatMoney(txn.amount), 12)
                  << column(txn.status, 12)
                  << note << "\n";
    }

    std::cout << "\n";
}
