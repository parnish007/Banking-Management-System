#include "user/transfer.h"
#include "auth/session.h"
#include "models/account.h"
#include "models/transaction.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <limits>

// ─────────────────────────────────────────────────────────────────────────────
// Helper: format a double as "$X,XXX.XX"
// ─────────────────────────────────────────────────────────────────────────────
static std::string formatMoney(double amount) {
    std::ostringstream raw;
    raw << std::fixed << std::setprecision(2) << amount;
    std::string s   = raw.str();
    std::size_t dot = s.find('.');
    std::string intPart  = s.substr(0, dot);
    std::string fracPart = s.substr(dot);
    int pos = static_cast<int>(intPart.size()) - 3;
    while (pos > 0) { intPart.insert(pos, ","); pos -= 3; }
    return "$" + intPart + fracPart;
}

// ─────────────────────────────────────────────────────────────────────────────
void Transfer::sendMoney(DB& db) {

    // ── 1. Get sender's account ───────────────────────────────────────────────
    SessionData session = Session::current();

    auto senderOpt = AccountModel::findByUserId(db, session.user_id);
    if (!senderOpt) {
        std::cout << "Error: could not load your account.\n";
        return;
    }
    Account sender = *senderOpt;   // copy — we'll re-fetch balance check later

    // ── 2. Prompt for recipient account number ────────────────────────────────
    std::string recipientAccNum;
    std::cout << "\nEnter recipient account number: ";
    std::cin >> recipientAccNum;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // ── 3. Look up recipient ──────────────────────────────────────────────────
    auto recipientOpt = AccountModel::findByAccountNumber(db, recipientAccNum);
    if (!recipientOpt) {
        std::cout << "Account not found.\n";
        return;
    }
    const Account& recipient = *recipientOpt;

    // ── 4a. Reject self-transfer ──────────────────────────────────────────────
    if (recipient.id == sender.id) {
        std::cout << "Cannot send money to yourself.\n";
        return;
    }

    // ── 4b. Prompt for amount ─────────────────────────────────────────────────
    double amount = 0.0;
    std::cout << "Enter amount: ";
    if (!(std::cin >> amount)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid amount.\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (amount <= 0.0) {
        std::cout << "Amount must be greater than zero.\n";
        return;
    }

    // ── 4c. Check funds — re-fetch balance in case it changed since login ─────
    senderOpt = AccountModel::findByUserId(db, session.user_id);
    if (!senderOpt) {
        std::cout << "Error: could not verify your balance.\n";
        return;
    }
    sender = *senderOpt;

    if (sender.balance < amount) {
        std::cout << "Insufficient funds. Your balance is "
                  << formatMoney(sender.balance) << ".\n";
        return;
    }

    // ── 5. Optional note ──────────────────────────────────────────────────────
    std::string note;
    std::cout << "Enter a note (optional — press Enter to skip): ";
    std::getline(std::cin, note);

    // ── 6. Create PENDING transaction — DO NOT touch any balances here ────────
    Transaction txn = TransactionModel::create(db, sender.id, recipient.id, amount, note);

    // ── 7. Confirm ────────────────────────────────────────────────────────────
    std::cout << "\nTransfer submitted.\n";
    std::cout << "Transaction #" << txn.id << " is pending admin approval.\n\n";
}
