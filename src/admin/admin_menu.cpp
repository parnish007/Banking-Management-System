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
#include "database/db.h"
#include "auth/session.h"

void AdminMenu::run(DB& db) {
    // Stub for Dev 4 implementation
}
