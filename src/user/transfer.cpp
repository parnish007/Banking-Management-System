#include "user/transfer.h"

#include "auth/session.h"
#include "models/account.h"
#include "models/transaction.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace {

std::string formatMoney(double amount) {
    std::ostringstream out;
    out << "$" << std::fixed << std::setprecision(2) << amount;
    return out.str();
}

void clearLine() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace

void Transfer::sendMoney(DB& db) {
    const SessionData session = Session::current();

    auto sender = AccountModel::findByUserId(db, session.user_id);
    if (!sender) {
        std::cout << "Error: could not load your account.\n";
        return;
    }

    std::cout << "\nEnter recipient account number: ";
    std::string recipient_account_number;
    std::getline(std::cin, recipient_account_number);

    auto recipient = AccountModel::findByAccountNumber(db, recipient_account_number);
    if (!recipient) {
        std::cout << "Account not found.\n";
        return;
    }

    if (recipient->id == sender->id) {
        std::cout << "Cannot send money to yourself.\n";
        return;
    }

    std::cout << "Enter amount: ";
    double amount = 0.0;
    if (!(std::cin >> amount)) {
        std::cin.clear();
        clearLine();
        std::cout << "Invalid amount.\n";
        return;
    }
    clearLine();

    if (amount <= 0.0) {
        std::cout << "Amount must be greater than zero.\n";
        return;
    }

    sender = AccountModel::findByUserId(db, session.user_id);
    if (!sender) {
        std::cout << "Error: could not verify your balance.\n";
        return;
    }

    if (sender->balance < amount) {
        std::cout << "Insufficient funds. Your balance is "
                  << formatMoney(sender->balance) << ".\n";
        return;
    }

    std::cout << "Enter a note (optional - press Enter to skip): ";
    std::string note;
    std::getline(std::cin, note);

    const Transaction txn = TransactionModel::create(
        db,
        sender->id,
        recipient->id,
        amount,
        note
    );

    std::cout << "\nTransfer submitted.\n";
    std::cout << "Transaction #" << txn.id << " is pending admin approval.\n\n";
}
