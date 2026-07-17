# Banking Management System Guide

This guide explains how to start the project, what the app can do, and how to
use each workflow from the console.

## 1. Requirements

Install or confirm these tools are available on `PATH`:

| Tool | Needed for |
|------|------------|
| `g++` / MinGW | C++ compilation |
| `cmake` | Build configuration |
| `mingw32-make` | Building with MinGW Makefiles |

SQLite is already bundled in `lib/sqlite3/`. No database server or extra package
install is required.

## 2. Build The Project

From the repository root:

```bat
build.bat
```

Manual build:

```bat
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
mingw32-make -j4
```

## 3. Run The App

From the repository root:

```bat
build\banking_system.exe
```

The app creates `banking.db` automatically on first run.

## 4. Default Admin Login

| Username | Password | Role |
|----------|----------|------|
| `admin` | `admin123` | Admin |

## 5. Main Menu

```text
=== Banking Management System ===
1. Login
2. Register
3. Exit
```

Use `Register` to create a regular user. Registration creates both the user row
and that user's bank account.

## 6. User Features

After a regular user logs in, the user menu provides:

```text
1. View Balance & Transaction History
2. Send Money
3. Logout
```

### View Balance And History

Shows:

| Field | Meaning |
|-------|---------|
| Account number | The logged-in user's account number |
| Balance | Current account balance |
| Transactions | All sent and received transactions, including pending ones |

Rows mark the current user's account with `(you)`.

### Send Money

The user enters:

1. Recipient account number, for example `ACC000002`
2. Amount
3. Optional note

The app validates that:

- The recipient account exists
- The sender is not sending to their own account
- The amount is greater than zero
- The sender has enough balance

Sending money creates a `pending` transaction. It does not move money
immediately. Balances change only after admin approval.

## 7. Admin Features

After the admin logs in, the admin menu provides:

```text
1. View Pending Transactions
2. Approve / Reject a Transaction
3. View All Users & Accounts
4. Logout
```

### View Pending Transactions

Lists all transfers waiting for review.

### Approve Or Reject

When approving a transaction, the app:

1. Re-checks the sender balance
2. Deducts from the sender
3. Credits the receiver
4. Marks the transaction as approved
5. Records which admin reviewed it

The balance updates and status update run inside a database transaction.

When rejecting a transaction, the app only changes the status to `rejected`.
No balances are changed.

### View All Users And Accounts

Shows every user, their role, account number, balance, and created date. Admin
users do not have bank accounts, so their account fields are shown as empty.

## 8. Demo Workflow

1. Run the app.
2. Register two users, for example `alice` and `bob`.
3. Give one user a test balance if your database has no existing funds.
4. Login as that funded user and send money to the other user's account number.
5. Logout.
6. Login as `admin` / `admin123`.
7. Open `Approve / Reject a Transaction`.
8. Approve the pending transaction.
9. Login as either user and view balance/history.

This project currently does not include a deposit screen. For local demos, seed
test balances directly in `banking.db` or reuse a database that already has a
funded user account.

## 9. Useful Files

| File | Purpose |
|------|---------|
| `README.md` | Project overview |
| `docs/build-guide.md` | Detailed build notes |
| `docs/architecture.md` | System design |
| `docs/database-schema.md` | Database tables |
| `docs/dev-assignments.md` | Developer ownership |
| `updatelog/dev1.md` | Database and model implementation log |
| `updatelog/dev2.md` | Auth implementation log |
| `updatelog/dev3.md` | User features implementation log |
| `updatelog/dev4.md` | Admin implementation log |

## 10. Troubleshooting

| Problem | Fix |
|---------|-----|
| `cmake` not found | Install CMake and add it to `PATH` |
| `g++` not found | Install MinGW and add `bin` to `PATH` |
| Admin login fails after changing password code | Delete `banking.db` and rerun so the seed data is regenerated |
| Transfer says insufficient funds | Seed a test balance or use an account with existing balance |
| Build folder has stale files | Delete `build/` and run `build.bat` again |
