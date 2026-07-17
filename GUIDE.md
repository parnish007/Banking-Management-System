# Banking Management System - Complete Guide

This guide explains how to run the project, how to use every feature, and how
the main parts of the code work internally. It is meant for both users testing
the console app and developers trying to understand the project structure.

## 1. Project Summary

Banking Management System is a C++17 console application backed by SQLite. It
supports two roles:

| Role | What They Can Do |
|------|------------------|
| User | Register, log in, view balance/history, submit transfer requests |
| Admin | Log in, view pending transfers, approve/reject transfers, view all users |

The most important business rule is that users do not move money directly.
Users create `pending` transactions. Admin approval is what actually updates
account balances.

## 2. Requirements

Install or confirm these tools are on `PATH`:

| Tool | Purpose |
|------|---------|
| `g++` / MinGW | Compiles C and C++ files |
| `cmake` | Generates the build files |
| `mingw32-make` | Runs the generated MinGW build |

SQLite is bundled in `lib/sqlite3/`, so no database server needs to be
installed.

## 3. Build And Run

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

Run:

```bat
build\banking_system.exe
```

The app creates `banking.db` automatically on first run. The database file is
ignored by Git because it is local runtime data.

## 4. Default Admin

The schema initializer seeds one admin user if no admin exists:

| Username | Password | Role |
|----------|----------|------|
| `admin` | `admin123` | Admin |

If you change password hashing code, delete `banking.db` before testing the
default admin again. The stored admin hash is generated when the database is
first created.

## 5. Main Menu

When the app starts, `src/main.cpp` opens the database, initializes the schema,
then shows:

```text
=== Banking Management System ===
1. Login
2. Register
3. Exit
```

### Login

Login asks for username and password. If credentials are correct, the app stores
a session and routes by role:

| Session Role | Menu Called |
|--------------|-------------|
| `admin` | `AdminMenu::run(db)` |
| `user` | `UserMenu::run(db)` |

Wrong usernames and wrong passwords both show the same generic error:

```text
Invalid username or password.
```

That is intentional. It prevents the app from revealing whether a username
exists.

### Register

Registration asks for:

1. Name
2. Username
3. Password

The app rejects empty username/password values and duplicate usernames. On
success, it creates both:

| Table | Row Created |
|-------|-------------|
| `users` | A regular user with role `user` |
| `accounts` | A bank account linked to that user |

The account number is printed immediately, for example:

```text
Registration successful! Your account number is: ACC000002
```

## 6. User Workflow

After a regular user logs in:

```text
=== Welcome, alice ===

1. View Balance & Transaction History
2. Send Money
3. Logout
```

### View Balance And Transaction History

This feature is implemented in:

| File | Responsibility |
|------|----------------|
| `include/user/user_account.h` | Declares `UserAccount::showBalanceAndHistory` |
| `src/user/user_account.cpp` | Fetches account and transactions, prints output |

The flow is:

1. Read the logged-in user from `Session::current()`.
2. Fetch that user's account using `AccountModel::findByUserId`.
3. Print the current account number and balance.
4. Fetch transactions using `TransactionModel::findByAccountId`.
5. Print all transactions where the account is either sender or receiver.

The current user's account is marked with `(you)` so direction is easy to read.

### Send Money

This feature is implemented in:

| File | Responsibility |
|------|----------------|
| `include/user/transfer.h` | Declares `Transfer::sendMoney` |
| `src/user/transfer.cpp` | Handles transfer input and validation |

The flow is:

1. Read the logged-in user from `Session::current()`.
2. Fetch the sender account.
3. Ask for the recipient account number.
4. Confirm the recipient account exists.
5. Reject transfers to the same account.
6. Ask for amount.
7. Reject invalid amounts and amounts greater than sender balance.
8. Ask for an optional note.
9. Create a `pending` transaction.

Important: `Transfer::sendMoney` does not update account balances. It only calls
`TransactionModel::create`. Balance changes are controlled by the admin approval
workflow.

### Logout

Logout calls:

```cpp
Session::end();
```

Then control returns to the main menu. Every menu must end the session before
returning so the next person at the terminal does not inherit the previous
login.

## 7. Admin Workflow

After the admin logs in:

```text
=== Admin Menu ===
1. View Pending Transactions
2. Approve / Reject a Transaction
3. View All Users & Accounts
4. Logout
```

### View Pending Transactions

Implemented in `Approval::listPending(db)`.

It calls `TransactionModel::findPending(db)` and displays transactions whose
status is `pending`.

### Approve A Transaction

Implemented in `Approval::reviewTransaction(db)`.

The approval flow is:

1. Show all pending transactions.
2. Ask which transaction ID to review.
3. Load the transaction.
4. Confirm it is still `pending`.
5. Load sender and receiver accounts.
6. Re-check sender balance.
7. Start a database transaction with `BEGIN`.
8. Deduct from sender.
9. Credit receiver.
10. Mark transaction `approved`.
11. Store `reviewed_by` and `reviewed_at`.
12. Commit with `COMMIT`.

The `BEGIN` / `COMMIT` block matters because approving a transfer changes three
things: sender balance, receiver balance, and transaction status. They must all
succeed together.

### Reject A Transaction

Rejecting only changes transaction status:

```text
pending -> rejected
```

No balances are changed because money never moved.

### View All Users And Accounts

Implemented in `UserMgmt::listAllUsers(db)`.

It shows every user, their role, account number, balance, and created date.
Admin users do not have normal bank accounts, so account fields are shown as
empty placeholders.

## 8. How Dev 1 Works: Database And Models

Dev 1 built the foundation that every later module uses.

### DB Wrapper

Files:

| File | Purpose |
|------|---------|
| `include/database/db.h` | Declares the `DB` class |
| `src/database/db.cpp` | Implements SQLite connection, execute, query |

The app does not use raw `sqlite3` calls all over the codebase. Instead, modules
use:

| Function | Purpose |
|----------|---------|
| `DB::execute(sql)` | Run SQL that does not return rows |
| `DB::execute(sql, binder)` | Run parameterized SQL with bound values |
| `DB::query(sql)` | Run SQL that returns rows |
| `DB::query(sql, binder)` | Run parameterized SELECT queries |
| `DB::lastInsertId()` | Get the last inserted row ID |

The binder-lambda pattern keeps SQL parameter binding close to the query while
still avoiding unsafe string concatenation.

### Schema Initialization

Files:

| File | Purpose |
|------|---------|
| `include/database/schema.h` | Declares `Schema::init` |
| `src/database/schema.cpp` | Creates tables and seeds admin |

`Schema::init(db)` creates three tables if they do not already exist:

| Table | Purpose |
|-------|---------|
| `users` | Login identity, password hash, role |
| `accounts` | Account number and balance for regular users |
| `transactions` | Transfer requests and review history |

It also seeds the default admin if no admin exists.

### Models

The model layer gives the rest of the app typed C++ functions instead of raw SQL.

| Model | Main Functions |
|-------|----------------|
| `UserModel` | `findById`, `findByUsername`, `create`, `findAll` |
| `AccountModel` | `create`, `findByUserId`, `findByAccountNumber`, `updateBalance`, `findAll` |
| `TransactionModel` | `create`, `findPending`, `findByAccountId`, `updateStatus` |

This separation keeps UI code readable. For example, auth code says
`UserModel::findByUsername(db, username)` instead of writing a SELECT statement
inside the login function.

## 9. How Dev 2 Works: Auth, Passwords, Sessions, Main Loop

Dev 2 connected the application together by implementing registration, login,
session state, and the main entry point.

### Password Hashing

Passwords are never stored as plain text. Registration calls:

```cpp
Password::hash(password)
```

Login calls:

```cpp
Password::verify(input_password, stored_hash)
```

The current project uses a minimal FNV-1a-style hash from Dev 1. This is fine for
the course project, but it is not production-grade password storage. A real
banking system would use a slow salted password hashing algorithm such as Argon2
or bcrypt.

### Registration Internals

`Auth::registerUser(db, name, username, password)` does this:

1. Reject empty username/password.
2. Call `UserModel::findByUsername` to check duplicates.
3. Hash the password.
4. Call `UserModel::create` with role `user`.
5. Call `AccountModel::create` for the new user.
6. Print the new account number.

The account number comes from `AccountModel::create`, which generates numbers
like `ACC000002`.

### Login Internals

`Auth::loginUser(db, username, password)` does this:

1. Look up the user by username.
2. Return `false` if no such user exists.
3. Verify password.
4. Return `false` if password does not match.
5. Build `SessionData`.
6. Start the session with `Session::start(session_data)`.

Admin sessions use `account_id = 0` because admins do not have normal customer
accounts. Regular user sessions store the user's real account id.

### SessionData

Declared in `include/auth/session.h`:

| Field | Meaning |
|-------|---------|
| `user_id` | `users.id` of the logged-in user |
| `username` | Display username |
| `role` | Either `user` or `admin` |
| `account_id` | `accounts.id` for users, `0` for admins |

### Session Namespace

Implemented in `src/auth/session.cpp`:

| Function | What It Does |
|----------|--------------|
| `Session::start(data)` | Stores session data and marks session active |
| `Session::end()` | Marks session inactive |
| `Session::isLoggedIn()` | Returns whether a session is active |
| `Session::current()` | Returns current session or throws if not logged in |

The session is intentionally in-memory only. It is not saved to the database or
disk. When the program exits, the session is gone.

### Main Loop

`src/main.cpp` owns the top-level routing:

1. Open `banking.db`.
2. Run `Schema::init(db)`.
3. Show main menu.
4. On login, call `Auth::loginUser`.
5. If session role is `admin`, call `AdminMenu::run(db)`.
6. Otherwise call `UserMenu::run(db)`.
7. When the menu returns, show the main menu again.

This keeps role routing in one file.

## 10. How Dev 3 Works: User Features

Dev 3 implemented the regular user side of the system.

| File | Role |
|------|------|
| `src/user/user_menu.cpp` | User menu loop |
| `src/user/user_account.cpp` | Balance and transaction history |
| `src/user/transfer.cpp` | Send money flow |

The user menu catches errors from balance/history and transfer operations so a
bad query does not kill the whole app.

The transfer flow deliberately creates only a pending transaction. This prevents
double-spending bugs where money might be deducted once during transfer creation
and again during admin approval.

## 11. How Dev 4 Works: Admin Features

Dev 4 implemented the admin side of the system.

| File | Role |
|------|------|
| `src/admin/admin_menu.cpp` | Admin menu loop |
| `src/admin/approval.cpp` | Pending list, approve, reject |
| `src/admin/user_mgmt.cpp` | User and account listing |

The most important correctness point is atomic approval. The app wraps approval
updates in a DB transaction so partial money movement does not happen.

## 12. Transaction Lifecycle

```text
User submits transfer
        |
        v
transactions.status = pending
        |
        v
Admin reviews transaction
        |
        +--> approve: sender balance decreases, receiver balance increases,
        |             status becomes approved
        |
        +--> reject:  balances stay unchanged, status becomes rejected
```

Transaction statuses are limited by the database schema:

| Status | Meaning |
|--------|---------|
| `pending` | Waiting for admin review |
| `approved` | Admin approved and balances were updated |
| `rejected` | Admin rejected and balances were not changed |

## 13. Demo Workflow

Use this flow to test the full app manually:

1. Build with `build.bat`.
2. Run `build\banking_system.exe`.
3. Register `alice`.
4. Register `bob`.
5. Add test funds to one account if your database has no funded users.
6. Login as the funded user.
7. Choose `Send Money`.
8. Enter the other user's account number.
9. Enter an amount and optional note.
10. Logout.
11. Login as `admin` / `admin123`.
12. Choose `Approve / Reject a Transaction`.
13. Approve the pending transfer.
14. Logout.
15. Login as either user and view transaction history.

There is currently no deposit screen. For demos, seed balance directly in
`banking.db` or use an account that already has funds.

## 14. Developer Map

| Area | Public Header | Implementation |
|------|---------------|----------------|
| Database wrapper | `include/database/db.h` | `src/database/db.cpp` |
| Schema | `include/database/schema.h` | `src/database/schema.cpp` |
| Users | `include/models/user.h` | `src/models/user.cpp` |
| Accounts | `include/models/account.h` | `src/models/account.cpp` |
| Transactions | `include/models/transaction.h` | `src/models/transaction.cpp` |
| Auth | `include/auth/auth.h` | `src/auth/auth.cpp` |
| Session | `include/auth/session.h` | `src/auth/session.cpp` |
| Passwords | `include/auth/password.h` | `src/auth/password.cpp` |
| User menu | `include/user/user_menu.h` | `src/user/user_menu.cpp` |
| Transfer | `include/user/transfer.h` | `src/user/transfer.cpp` |
| User account view | `include/user/user_account.h` | `src/user/user_account.cpp` |
| Admin menu | `include/admin/admin_menu.h` | `src/admin/admin_menu.cpp` |
| Approval | `include/admin/approval.h` | `src/admin/approval.cpp` |
| User management | `include/admin/user_mgmt.h` | `src/admin/user_mgmt.cpp` |

## 15. Common Problems

| Problem | Cause | Fix |
|---------|-------|-----|
| `cmake` not found | CMake is not on `PATH` | Install CMake or add it to `PATH` |
| `g++` not found | MinGW is not on `PATH` | Add MinGW `bin` folder to `PATH` |
| Admin login fails | Old `banking.db` has hashes from old password logic | Delete `banking.db` and rerun |
| Transfer says insufficient funds | Sender balance is zero | Seed test balance before transfer |
| Newly changed files do not seem compiled | Build cache is stale | Delete `build/` and run `build.bat` |
| `Session::current()` throws | Code called it without active login | Call only after successful login |

## 16. Verification Commands

Configure:

```bat
cmake -S . -B build
```

Build:

```bat
cmake --build build
```

Run configured tests:

```bat
ctest --test-dir build --output-on-failure
```

At the moment, CTest is configured but no automated tests are registered.

Smoke test manually:

```bat
build\banking_system.exe
```

Then choose `3. Exit`. The app should print `Goodbye.`
