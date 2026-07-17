// =============================================================================
// OWNER : Dev 3 — User Features
// FILE  : include/user/transfer.h
// ABOUT : Handles the "Send Money" flow for a regular user.
//         Creates a PENDING transaction — balance is NOT changed until admin
//         approves. Sender must have sufficient balance to submit.
//
// MUST PROVIDE:
//   namespace Transfer
//     - void sendMoney(DB& db)
//         → prompts: recipient account number, amount, optional note
//         → validates: recipient exists, not sending to self, balance >= amount
//         → creates a Transaction with status "pending"
//         → prints confirmation with transaction id
//
// DEPENDENCIES: db.h, models/account.h, models/transaction.h, auth/session.h
// =============================================================================
#pragma once
#include "database/db.h"

namespace Transfer {
    void sendMoney(DB& db);
}
