// =============================================================================
// OWNER : Dev 4 — Admin Panel
// FILE  : include/admin/user_mgmt.h
// ABOUT : Admin view of all registered users and their account details.
//
// MUST PROVIDE:
//   namespace UserMgmt
//     - void listAllUsers(DB& db)
//         → prints a table of all users:
//             columns: ID | Name | Username | Account No. | Balance | Joined
//
// DEPENDENCIES: db.h, models/user.h, models/account.h
// =============================================================================
#pragma once

class DB;

namespace UserMgmt {
    void listAllUsers(DB& db);
}
