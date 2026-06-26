// =============================================================================
// OWNER : Dev 3 — User Features
// FILE  : src/user/user_menu.cpp
// ABOUT : Implement UserMenu::run() declared in include/user/user_menu.h.
//
// TASKS:
//   [ ] Print welcome message with the logged-in user's name
//   [ ] Loop showing:
//         === User Menu ===
//         1. View Balance & History
//         2. Send Money
//         3. Logout
//         > _
//   [ ] On choice 1: call UserAccount::showBalanceAndHistory(db)
//   [ ] On choice 2: call Transfer::sendMoney(db)
//   [ ] On choice 3: call Session::end(), print "Logged out.", break loop
//   [ ] Handle invalid input gracefully (print "Invalid option, try again.")
// =============================================================================
#include "user/user_menu.h"
#include "database/db.h"
#include "auth/session.h"

void UserMenu::run(DB& db) {
    // Stub for Dev 3 implementation
}
