# Dev 3 - Update Log

**Role:** User Features  
**Status:** Complete

## Files Delivered

| File | Status |
|------|--------|
| `include/user/user_account.h` + `src/user/user_account.cpp` | Done |
| `include/user/transfer.h` + `src/user/transfer.cpp` | Done |
| `include/user/user_menu.h` + `src/user/user_menu.cpp` | Done |

## What Was Implemented

### User Account

`UserAccount::showBalanceAndHistory(db)` displays the logged-in user's account
number, current balance, and full transaction history. It re-fetches account
data each time so balances are current, includes pending transactions, and marks
the user's own account with `(you)` in transaction rows.

### Transfer

`Transfer::sendMoney(db)` lets a logged-in user submit a transfer request. It
validates the recipient account, blocks self-transfers, rejects invalid or
unfunded amounts, accepts an optional note, and creates a pending transaction.
It does not update balances; balances only change after admin approval.

### User Menu

`UserMenu::run(db)` provides the regular user menu:

```text
1. View Balance & Transaction History
2. Send Money
3. Logout
```

Logout calls `Session::end()` before returning to the main login loop.

## Integration Fixes

After merging PR #5, the implementation was moved from root-level files into the
project's existing `include/user` and `src/user` structure. Transaction history
account-number display was also fixed to read account numbers from the database
by account id instead of deriving them from ids.
