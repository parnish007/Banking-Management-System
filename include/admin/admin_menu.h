// =============================================================================
// OWNER : Dev 4 — Admin Panel
// FILE  : include/admin/admin_menu.h
// ABOUT : Top-level menu loop for a logged-in admin.
//         Called from main.cpp immediately after a successful admin login.
//
// MUST PROVIDE:
//   namespace AdminMenu
//     - void run(DB& db)
//         → shows the admin menu in a loop until admin chooses Logout:
//             1. View Pending Transactions
//             2. Approve / Reject a Transaction
//             3. View All Users & Accounts
//             4. Logout
//
// DEPENDENCIES: db.h, auth/session.h, admin/approval.h, admin/user_mgmt.h
// =============================================================================
#pragma once

class DB;

namespace AdminMenu {
    void run(DB& db);
}
