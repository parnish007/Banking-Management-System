// =============================================================================
// OWNER : Dev 3 — User Features
// FILE  : include/user/user_menu.h
// ABOUT : Top-level menu loop for a logged-in regular user.
//         Called from main.cpp immediately after a successful user login.
//
// MUST PROVIDE:
//   namespace UserMenu
//     - void run(DB& db)
//         → shows the user menu in a loop until the user chooses Logout:
//             1. View Balance & Transaction History
//             2. Send Money
//             3. Logout
//
// DEPENDENCIES: db.h, auth/session.h, user/transfer.h, user/user_account.h
// =============================================================================
#pragma once

class DB;

namespace UserMenu {
    void run(DB& db);
}
