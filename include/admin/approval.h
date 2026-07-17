// =============================================================================
// OWNER : Dev 4 — Admin Panel
// FILE  : include/admin/approval.h
// ABOUT : Admin workflow for reviewing PENDING transactions.
//         Approving must atomically update both account balances + status.
//         Rejecting only changes the status — no balance changes.
//
// MUST PROVIDE:
//   namespace Approval
//     - void listPending(DB& db)
//         → prints a table of all PENDING transactions:
//             columns: ID | Date | From Account | To Account | Amount | Note
//
//     - void reviewTransaction(DB& db)
//         → prompts admin to enter a transaction ID
//         → shows transaction details
//         → asks: Approve (A) / Reject (R) / Cancel (C)
//         → on Approve: deducts sender balance, credits receiver, marks APPROVED
//         → on Reject:  marks REJECTED, no balance change
//         → in both cases records reviewed_by = current admin id + reviewed_at
//
// DEPENDENCIES: db.h, models/account.h, models/transaction.h, auth/session.h
// =============================================================================
#pragma once

class DB;

namespace Approval {
    void listPending(DB& db);
    void reviewTransaction(DB& db);
}
