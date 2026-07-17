#include "user/user_account.h"
#include "auth/session.h"
#include "models/account.h"
#include "models/transaction.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Helper: format a double as a dollar string  e.g. 1250.5 → "$1,250.50"
// ─────────────────────────────────────────────────────────────────────────────
static std::string formatMoney(double amount) {
    // Build the raw decimal string first
    std::ostringstream raw;
    raw << std::fixed << std::setprecision(2) << amount;
    std::string s = raw.str();

    // Locate the decimal point
    std::size_t dot = s.find('.');
    std::string intPart  = s.substr(0, dot);
    std::string fracPart = s.substr(dot);      // ".XX"

    // Insert thousands separators
    int insertPos = static_cast<int>(intPart.size()) - 3;
    while (insertPos > 0) {
        intPart.insert(insertPos, ",");
        insertPos -= 3;
    }

    return "$" + intPart + fracPart;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: left-pad a string to a fixed width (truncates if too long)
// ─────────────────────────────────────────────────────────────────────────────
static std::string col(const std::string& s, int width) {
    if (static_cast<int>(s.size()) >= width)
        return s.substr(0, width);
    return s + std::string(width - s.size(), ' ');
}

// ─────────────────────────────────────────────────────────────────────────────
void UserAccount::showBalanceAndHistory(DB& db) {

    // ── 1. Fetch session ──────────────────────────────────────────────────────
    SessionData session = Session::current();

    // ── 2. Re-fetch account (never use cached balance) ────────────────────────
    auto accOpt = AccountModel::findByUserId(db, session.user_id);
    if (!accOpt) {
        std::cout << "Error: could not load your account. Please contact an admin.\n";
        return;
    }
    const Account& acc = *accOpt;

    // ── 3. Print balance banner ───────────────────────────────────────────────
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════╗\n";
    std::cout << "  Account   : " << acc.account_number << "\n";
    std::cout << "  Balance   : " << formatMoney(acc.balance) << "\n";
    std::cout << "╚══════════════════════════════════╝\n\n";

    // ── 4. Fetch transactions ─────────────────────────────────────────────────
    std::vector<Transaction> txns = TransactionModel::findByAccountId(db, acc.id);

    if (txns.empty()) {
        std::cout << "No transactions yet.\n\n";
        return;
    }

    // ── 5. Print transaction table ────────────────────────────────────────────
    // Column widths: ID(4) Date(20) From(16) To(16) Amount(12) Status(10) Note(20)
    std::cout << col("ID",     4)  << "  "
              << col("Date",   19) << "  "
              << col("From",   14) << "  "
              << col("To",     14) << "  "
              << col("Amount", 11) << "  "
              << col("Status", 10) << "  "
              << "Note\n";

    std::cout << std::string(4,  '-') << "  "
              << std::string(19, '-') << "  "
              << std::string(14, '-') << "  "
              << std::string(14, '-') << "  "
              << std::string(11, '-') << "  "
              << std::string(10, '-') << "  "
              << std::string(20, '-') << "\n";

    for (const Transaction& t : txns) {
        // Annotate the user's own account with "(you)"
        auto label = [&](int acct_id, const std::string& acct_num) -> std::string {
            return (acct_id == acc.id) ? acct_num + "(you)" : acct_num;
        };

        // We need account numbers from account ids. Use a small inline fetch.
        // Dev 1 only exposes findByUserId / findByAccountNumber, so we look up by id
        // indirectly: store account_number in Transaction is not available, so we
        // format using ACC%06d just like AccountModel::create does — deterministic.
        auto accNumFrom = [](int id) -> std::string {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "ACC%06d", id);
            return std::string(buf);
        };

        // from_account_id and to_account_id map 1-to-1 to user_id in this schema
        // (account number = ACC + zero-padded user_id). We derive the display string.
        std::string fromStr = label(t.from_account_id, accNumFrom(t.from_account_id));
        std::string toStr   = label(t.to_account_id,   accNumFrom(t.to_account_id));

        std::string idStr     = std::to_string(t.id);
        std::string amountStr = formatMoney(t.amount);
        std::string noteStr   = t.note.empty() ? "—" : t.note;

        std::cout << col(idStr,       4)  << "  "
                  << col(t.created_at, 19) << "  "
                  << col(fromStr,     14) << "  "
                  << col(toStr,       14) << "  "
                  << col(amountStr,   11) << "  "
                  << col(t.status,    10) << "  "
                  << noteStr << "\n";
    }

    std::cout << "\n";
}
