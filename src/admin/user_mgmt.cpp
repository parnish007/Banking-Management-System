// =============================================================================
// OWNER : Dev 4 — Admin Panel
// FILE  : src/admin/user_mgmt.cpp
// ABOUT : Implement UserMgmt::listAllUsers() declared in
//         include/admin/user_mgmt.h.
//
// TASKS:
//   [ ] Fetch all users (UserModel::findAll)
//   [ ] For each user, fetch their account (AccountModel::findByUserId)
//   [ ] Print table:
//         ID | Name          | Username | Account No. | Balance    | Role  | Joined
//         ---|---------------|----------|-------------|------------|-------|-------------------
//         1  | Administrator | admin    | —           | —          | admin | 2024-01-01 00:00
//         2  | Alice Johnson | alice    | ACC000001   | $1,000.00  | user  | 2024-05-01 10:00
//
//   [ ] Admin accounts have no balance/account — display "—" for those columns
// =============================================================================



#include "admin/user_mgmt.h"
#include "models/user.h"
#include "models/account.h"
#include <iostream>
#include <iomanip>
#include <vector>

void UserMgmt::listAllUsers(DB& db) {
    auto users = UserModel::findAll(db);
    
    std::cout << "\nID   | Name            | Username | Account No. | Balance    | Role  | Joined\n";
    std::cout << "-----|-----------------|----------|-------------|------------|-------|-------------------\n";
    
    if (users.size() == 0) {
        return;
    }

    for (const auto& user : users) {
        std::cout << std::left 
                  << std::setw(5)  << user.id 
                  << "| " << std::setw(16) << user.name
                  << "| " << std::setw(9)  << user.username;
                  
        if (user.role == "admin") {
            std::cout << "| " << std::setw(12) << "—"
                      << "| " << std::setw(11) << "—";
        } else {
            auto acc = AccountModel::findByUserId(db, user.id);

            if (acc.has_value()) {
                std::cout << "| " << std::setw(12) << acc->account_number
                          << "| $" << std::fixed << std::setprecision(2) << std::setw(9) << acc->balance;
            } else {
                std::cout << "| " << std::setw(12) << "—"
                          << "| " << std::setw(11) << "—";
            }
        }
        
        std::cout << "| " << std::setw(6) << user.role
                  << "| " << user.created_at << "\n";
    }
    std::cout << "---------------------------------------------------------------------------------------------------------\n";
}