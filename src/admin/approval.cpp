// =============================================================================
// OWNER : Dev 4 — Admin Panel
// FILE  : src/admin/approval.cpp
// ABOUT : Implement Approval::listPending() and Approval::reviewTransaction().
//
// TASKS:
//   [ ] listPending(db)
//         → fetch all pending transactions (TransactionModel::findPending)
//         → if none: print "No pending transactions."
//         → else print table:
//             ID | Date                | From Acc   | To Acc     | Amount  | Note
//             ---|---------------------|------------|------------|---------|-----
//             3  | 2024-05-01 10:23:00 | ACC000001  | ACC000002  | $500.00 | rent
//
//   [ ] reviewTransaction(db)
//         → call listPending(db) first so admin sees what's available
//         → prompt "Enter Transaction ID to review (0 to cancel): "
//         → fetch the specific transaction, verify it is still "pending"
//         → show transaction details
//         → prompt "Action: (A)pprove / (R)eject / (C)ancel: "
//         → on Approve:
//             - check sender still has sufficient balance
//             - AccountModel::updateBalance(sender, sender.balance - amount)
//             - AccountModel::updateBalance(receiver, receiver.balance + amount)
//             - TransactionModel::updateStatus(id, "approved", admin_id)
//             - print "Transaction #X approved."
//         → on Reject:
//             - TransactionModel::updateStatus(id, "rejected", admin_id)
//             - print "Transaction #X rejected."
//
// IMPORTANT: Approve must update BOTH balances AND status atomically.
//            Use db.execute("BEGIN") / db.execute("COMMIT") to wrap them.
// =============================================================================
#include "admin/approval.h"
#include "auth/session.h"
#include "models/account.h"
#include "models/transaction.h"
#include "models/user.h"
#include <cctype>
#include <cstddef>
#include <iostream>
#include <iomanip>
#include <optional>
#include <string>
#include <vector>

static std::optional<Account> findAccountById(DB& db, int account_id) {
    std::vector<DB::Row> rows = db.query(
        "SELECT * FROM accounts WHERE id = ? LIMIT 1;",
        [account_id](sqlite3_stmt* s) {
            sqlite3_bind_int(s, 1, account_id);
        }
    );

    if (rows.size() == 0) {
        return std::nullopt;
    }

    const DB::Row& r = rows[0];
    Account a;
    a.id             = std::stoi(r.at("id"));
    a.user_id        = std::stoi(r.at("user_id"));
    a.account_number = r.at("account_number");
    a.balance        = std::stod(r.at("balance"));
    a.created_at     = r.at("created_at");
    return a;
}

static Transaction rowToTransaction(const DB::Row& r) {
    Transaction t;
    t.id              = std::stoi(r.at("id"));
    t.from_account_id = std::stoi(r.at("from_account_id"));
    t.to_account_id   = std::stoi(r.at("to_account_id"));
    t.amount          = std::stod(r.at("amount"));
    t.status          = r.at("status");
    t.note            = r.at("note");
    t.created_at      = r.at("created_at");
    t.reviewed_by     = r.at("reviewed_by").empty() ? 0 : std::stoi(r.at("reviewed_by"));
    t.reviewed_at     = r.at("reviewed_at");
    return t;
}

static std::optional<Transaction> findTransactionById(DB& db, int txn_id) {
    std::vector<DB::Row> rows = db.query(
        "SELECT * FROM transactions WHERE id = ? LIMIT 1;",
        [txn_id](sqlite3_stmt* s) {
            sqlite3_bind_int(s, 1, txn_id);
        }
    );

    if (rows.size() == 0) {
        return std::nullopt;
    }

    return rowToTransaction(rows[0]);
}


namespace Approval {

void listPending(DB& db) {
    std::vector<Transaction> pending = TransactionModel::findPending(db);

    if (pending.size() == 0) {
        std::cout << "No pending transactions." << std::endl;
        return;
    }

    std::cout << "\nID    | Date                | From Account | To Account   | Amount     | Note\n";
    std::cout << "------+---------------------+--------------+--------------+------------+---------------------------\n";

    for (std::size_t idx = 0; idx < pending.size(); ++idx) {
        const Transaction& txn = pending[idx];
        std::optional<Account> from_account = findAccountById(db, txn.from_account_id);
        std::optional<Account> to_account   = findAccountById(db, txn.to_account_id);

        std::string from_number = from_account.has_value() ? from_account->account_number : "UNKNOWN";
        std::string to_number   = to_account.has_value()   ? to_account->account_number   : "UNKNOWN";

        std::cout << std::left
                  << std::setw(6)  << txn.id
                  << std::setw(21) << txn.created_at
                  << std::setw(14) << from_number
                  << std::setw(14) << to_number
                  << std::left << "$" << std::right << std::setw(10) << std::fixed << std::setprecision(2) << txn.amount
                  << std::left << " | " << txn.note << "\n";
    }
}

void reviewTransaction(DB& db) {
    listPending(db);

    auto pending = TransactionModel::findPending(db);
    if (pending.empty()) {
        return;
    }

    std::cout << "\nEnter Transaction ID to review (0 to cancel): ";
    std::string input;
    std::getline(std::cin, input);

    int txn_id = 0;
    try {
        txn_id = std::stoi(input);
    } catch (...) {
        std::cout << "Invalid transaction ID." << std::endl;
        return;
    }

    if (txn_id == 0) {
        std::cout << "Canceled." << std::endl;
        return;
    }

    auto txn_opt = findTransactionById(db, txn_id);
    if (!txn_opt.has_value()) {
        std::cout << "Transaction not found." << std::endl;
        return;
    }

    auto txn = txn_opt.value();
    if (txn.status != "pending") {
        std::cout << "Transaction #" << txn.id << " is not pending." << std::endl;
        return;
    }

    auto from_account = findAccountById(db, txn.from_account_id);
    auto to_account   = findAccountById(db, txn.to_account_id);

    if (!from_account.has_value() || !to_account.has_value()) {
        std::cout << "Unable to load account details for transaction." << std::endl;
        return;
    }

    std::cout << "\nTransaction #" << txn.id << " details:\n";
    std::cout << "Date       : " << txn.created_at << "\n";
    std::cout << "From       : " << from_account->account_number << "\n";
    std::cout << "To         : " << to_account->account_number << "\n";
    std::cout << "Amount     : $" << std::fixed << std::setprecision(2) << txn.amount << "\n";
    std::cout << "Note       : " << txn.note << "\n";
    std::cout << "Status     : " << txn.status << "\n";

    std::cout << "\nAction: (A)pprove / (R)eject / (C)ancel: ";
    std::getline(std::cin, input);

    if (input.empty()) {
        std::cout << "No action selected." << std::endl;
        return;
    }

    char action = input[0];
    if (action >= 'a' && action <= 'z') {
        action = static_cast<char>(action - 'a' + 'A');
    }
    if (action == 'C') {
        std::cout << "Canceled." << std::endl;
        return;
    }

    auto session = Session::current();

    if (action == 'A') {
        if (from_account->balance < txn.amount) {
            std::cout << "Insufficient sender balance to approve transaction." << std::endl;
            return;
        }

        try {
            db.execute("BEGIN;");
            AccountModel::updateBalance(db, from_account->id, from_account->balance - txn.amount);
            AccountModel::updateBalance(db, to_account->id,   to_account->balance + txn.amount);
            TransactionModel::updateStatus(db, txn.id, "approved", session.user_id);
            db.execute("COMMIT;");
            std::cout << "Transaction #" << txn.id << " approved." << std::endl;
        } catch (...) {
            try {
                db.execute("ROLLBACK;");
            } catch (...) {
                // ignore rollback failures when already handling an exception
            }
            std::cout << "Failed to approve transaction. Changes were rolled back." << std::endl;
        }

    } else if (action == 'R') {
        if (TransactionModel::updateStatus(db, txn.id, "rejected", session.user_id)) {
            std::cout << "Transaction #" << txn.id << " rejected." << std::endl;
        } else {
            std::cout << "Failed to reject transaction." << std::endl;
        }
    } else {
        std::cout << "Invalid action." << std::endl;
    }
}

} // namespace Approval
