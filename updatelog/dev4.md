# Dev 4 — Update Log

**Role:** Admin Panel
**Date:** 2026-07-17
**Status:** Complete

---

## What Was Implemented

### 1. Admin Menu Loop

**Files:** `src/admin/admin_menu.cpp`

The admin menu now provides a complete console loop for authenticated administrators. It reads the selected action, routes to the approval or user-management workflow, and logs the admin out cleanly when they choose to exit.

**Design decision — why this loop is simple and explicit:**
The project’s console UI is intentionally procedural rather than event-driven. A straight `while (true)` loop keeps the flow easy to follow for the next developer and makes the menu behavior obvious in the terminal.

| Function              | What it does                                                                           |
| --------------------- | -------------------------------------------------------------------------------------- |
| `AdminMenu::run(DB&)` | Shows the admin menu and routes to pending-transaction review, user listing, or logout |

### 2. Pending Transaction Review and Approval

**Files:** `src/admin/approval.cpp`

The approval module now lists all pending transactions, allows the admin to review a specific transfer, and handles both approve and reject actions. Approved transfers update both account balances and the transaction status inside a database transaction, while rejections only change the transaction state.

**Design decision — why transaction approval is transactional:**
The bank balance update must be atomic. If the sender balance is reduced but the receiver balance or status update fails, the system would become inconsistent. Wrapping the update in `BEGIN` / `COMMIT` ensures the balances and status either all succeed or all roll back together.

| Function                           | SQL / Logic                                                                                                          |
| ---------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| `Approval::listPending(DB&)`       | Reads pending transactions from `TransactionModel::findPending()` and prints a readable table                        |
| `Approval::reviewTransaction(DB&)` | Prompts for a transaction ID, verifies the record is still pending, shows details, and either approves or rejects it |

### 3. User and Account Listing

**Files:** `src/admin/user_mgmt.cpp`

The user-management view now prints all registered users and their linked account details when available. Admin users are shown with no account data, while normal users display their account number and current balance.

**Design decision — why this view is read-only:**
The admin panel is a management surface rather than an editing surface. The code only reads from the database and formats the results for the console so it stays consistent with the project’s layered architecture.

| Function                      | SQL / Logic                                                                                             |
| ----------------------------- | ------------------------------------------------------------------------------------------------------- |
| `UserMgmt::listAllUsers(DB&)` | Reads every user via `UserModel::findAll()` and fetches each account via `AccountModel::findByUserId()` |

---

## Libraries Used

### Standard Library Headers

| Header       | Why it was used                                   |
| ------------ | ------------------------------------------------- |
| `<iostream>` | Console output and prompt handling                |
| `<iomanip>`  | Column formatting for the admin tables            |
| `<string>`   | Menu input and text formatting                    |
| `<vector>`   | Storing the transaction and user lists            |
| `<optional>` | Safe account lookup handling                      |
| `<cctype>`   | Normalising user input for approve/reject actions |

---

## What the Next Dev Needs From You

The admin panel depends on the contracts already established by Dev 1 and Dev 2:

- `Session::current()` must continue to provide the logged-in admin’s `user_id`, `username`, `role`, and `account_id`
- `UserModel::findAll()` and `AccountModel::findByUserId()` must keep returning the data needed by the user-management table
- `TransactionModel::findPending()`, `TransactionModel::updateStatus()`, and `AccountModel::updateBalance()` must remain available for the approval workflow

No additional contract changes were introduced for Dev 3 or Dev 4 beyond the existing interface.

---

## Verified Output

The admin menu and user-management modules were updated and checked against the repository’s existing DB/model interfaces.

The environment did not expose a working CMake/MinGW toolchain, so a full build could not be completed here. The attempted verification output was:

```text
cmake : The term 'cmake' is not recognized as the name of a cmdlet, function,
script file, or operable program.
```
