// =============================================================================
// OWNER : Dev 3 — User Features
// FILE  : include/user/user_account.h
// ABOUT : Displays the current user's account balance and full transaction
//         history (sent and received), including pending ones.
//
// MUST PROVIDE:
//   namespace UserAccount
//     - void showBalanceAndHistory(DB& db)
//         → prints account number, current balance
//         → prints a table of all transactions involving this account:
//             columns: ID | Date | From | To | Amount | Status
//
// DEPENDENCIES: db.h, models/account.h, models/transaction.h, auth/session.h
// =============================================================================
#pragma once
#include "database/db.h"

namespace UserAccount {
    void showBalanceAndHistory(DB& db);
}
