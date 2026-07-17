# Dev 3 — Update Log

**Role:** User Features  
**Date:** 2026-06-23  
**Status:** Complete ✅

---

## Files Delivered

| File | Status |
|------|--------|
| `include/user/user_account.h` + `src/user/user_account.cpp` | ✅ Done |
| `include/user/transfer.h` + `src/user/transfer.cpp` | ✅ Done |
| `include/user/user_menu.h` + `src/user/user_menu.cpp` | ✅ Done |

---

## What Was Implemented

### 1. `UserAccount::showBalanceAndHistory(db)` — `user_account.cpp`

Displays the logged-in user's account balance followed by their complete transaction history.

**Flow:**
1. Reads `session.user_id` from `Session::current()`.
2. Calls `AccountModel::findByUserId(db, user_id)` — re-fetches every call so the balance is always current (never stale from login time).
3. Prints a bordered balance banner showing account number and formatted balance.
4. Calls `TransactionModel::findByAccountId(db, acc.id)` to get all transactions (incoming and outgoing).
5. If the list is empty, prints `"No transactions yet."` and returns.
6. Otherwise prints a fixed-width table with columns: `ID | Date | From | To | Amount | Status | Note`.
7. The user's own account number is annotated with `(you)` in the `From` and `To` columns so they can immediately see whether they sent or received money.
8. Pending transactions are included — the user sees the full picture.

**Design decisions:**
- Account numbers are derived from account IDs using the same `ACC%06d` formula as `AccountModel::create`, which is deterministic — no extra DB lookup needed to get the account number string from an account ID.
- A local `formatMoney()` helper produces `$X,XXX.XX` output with thousands separators.
- A `col()` helper left-pads or truncates strings to a fixed column width so the table aligns cleanly regardless of data length.

---

### 2. `Transfer::sendMoney(db)` — `transfer.cpp`

Guides the logged-in user through the send-money flow and creates a `PENDING` transaction.

**Flow:**
1. Fetches sender's account via `AccountModel::findByUserId`.
2. Prompts for recipient account number (e.g. `ACC000005`).
3. Looks up recipient via `AccountModel::findByAccountNumber`. Stops with `"Account not found."` if not found — the amount prompt is never shown.
4. Rejects self-transfers with `"Cannot send money to yourself."`.
5. Prompts for amount. Validates `amount > 0`.
6. **Re-fetches the sender's balance** right before the funds check — another transaction might have been approved since login, changing the balance. Stops with `"Insufficient funds. Your balance is $X.XX"` if funds are short.
7. Prompts for an optional note (Enter to skip → empty string).
8. Calls `TransactionModel::create(db, sender.id, recipient.id, amount, note)` — status defaults to `"pending"` inside the model.
9. Prints `"Transaction #N is pending admin approval."`.

**Critical rule followed:** **No account balances are modified.** Balances only change when Dev 4's `Approval::reviewTransaction` approves the transaction. Touching balances here would cause double-spending.

**Input safety:**
- Non-numeric input for amount is caught by `std::cin` fail state — cleared and rejected gracefully.
- `std::cin.ignore()` is called after every `>>` read to clear the newline before `std::getline`.

---

### 3. `UserMenu::run(db)` — `user_menu.cpp`

The interactive console loop for a regular logged-in user.

**Menu:**
```
=== Welcome, Alice ===

1. View Balance & Transaction History
2. Send Money
3. Logout

> _
```

**Behaviour:**
- Loops indefinitely until the user selects `3. Logout`.
- On logout: calls `Session::end()`, prints `"Logged out."`, then `return`s — control goes back to `main.cpp`'s login loop.
- Non-integer input is caught via `std::cin` fail-state check, cleared, and prints `"Invalid option. Try again."` — no crash.
- Each sub-function call is wrapped in `try/catch(std::exception&)` so a DB error in `showBalanceAndHistory` or `sendMoney` prints a friendly message and returns to the menu instead of propagating up.

---

## Libraries Used

| Header | Why |
|--------|-----|
| `<iostream>` | `std::cout` / `std::cin` for all I/O |
| `<string>` | All text fields and user input |
| `<iomanip>` | `std::fixed`, `std::setprecision` for money formatting |
| `<sstream>` | `std::ostringstream` inside `formatMoney()` |
| `<limits>` | `std::numeric_limits<std::streamsize>::max()` for `cin.ignore()` |
| `<vector>` | Return type of `TransactionModel::findByAccountId` |
| `<cstdio>` | `std::snprintf` for account-number derivation in `user_account.cpp` |

No new external dependencies. All logic builds on Dev 1 and Dev 2's existing interfaces.

---

## Contract Delivered to `main.cpp` (Dev 2)

```cpp
// After a successful user login, Dev 2 calls:
#include "user/user_menu.h"
UserMenu::run(db);
// When this returns, the user has logged out and Session::end() has been called.
```

---

## What Dev 4 Needs to Know

- `Transfer::sendMoney` creates transactions with `status = "pending"` — Dev 4's `Approval::reviewTransaction` is responsible for changing status to `"approved"` or `"rejected"` and updating both account balances atomically.
- Dev 3 never calls `AccountModel::updateBalance`. That is exclusively Dev 4's domain.
