// =============================================================================
// OWNER : Dev 4 — Admin Panel
// FILE  : src/admin/admin_menu.cpp
// ABOUT : Implement AdminMenu::run() declared in include/admin/admin_menu.h.
//
// TASKS:
//   [ ] Print "Welcome, Admin <name>"
//   [ ] Loop showing:
//         === Admin Menu ===
//         1. View Pending Transactions
//         2. Approve / Reject a Transaction
//         3. View All Users & Accounts
//         4. Logout
//         > _
//   [ ] On choice 1: call Approval::listPending(db)
//   [ ] On choice 2: call Approval::reviewTransaction(db)
//   [ ] On choice 3: call UserMgmt::listAllUsers(db)
//   [ ] On choice 4: call Session::end(), print "Logged out.", break loop
//   [ ] Handle invalid input gracefully
// =============================================================================
#include "admin/admin_menu.h"
#include "admin/approval.h"
#include "admin/user_mgmt.h"
#include "database/db.h"
#include "auth/session.h"
#include <iostream>
#include <string>

static std::string trim(const std::string& input) {
    const auto start = input.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = input.find_last_not_of(" \t\r\n");
    return input.substr(start, end - start + 1);
}

void AdminMenu::run(DB& db) {
    auto session = Session::current();
    std::cout << "\nWelcome, Admin " << session.username << "\n";

    while (true) {
        std::cout << "\n=== Admin Menu ===\n";
        std::cout << "1. View Pending Transactions\n";
        std::cout << "2. Approve / Reject a Transaction\n";
        std::cout << "3. View All Users & Accounts\n";
        std::cout << "4. Logout\n";
        std::cout << "> ";

        std::string choice;
        std::getline(std::cin, choice);
        choice = trim(choice);

        if (choice == "1") {
            Approval::listPending(db);
        } else if (choice == "2") {
            Approval::reviewTransaction(db);
        } else if (choice == "3") {
            UserMgmt::listAllUsers(db);
        } else if (choice == "4") {
            Session::end();
            std::cout << "Logged out." << std::endl;
            break;
        } else {
            std::cout << "Invalid option. Try again." << std::endl;
        }
    }
}
