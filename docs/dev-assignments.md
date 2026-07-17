# Developer Assignments

## Build Order — Read This First

```
Dev 1  →  Dev 2  →  Dev 3 ┐
                           ├── parallel
                   Dev 4  ┘
```

- **Dev 1** must finish before anyone writes a single line of logic code.
  Everyone's code calls Dev 1's models and DB class.
- **Dev 2** must finish before Dev 3 or Dev 4 start, because both need
  `Session::current()` to know who is logged in.
- **Dev 3 and Dev 4** are fully independent of each other and can work at the
  same time once Dev 2 is done.

If you start before your dependency is done, you will write code that won't
link and you'll have to rewrite it.

---

## Dev 1 — Database & Models ✅ Complete

> See [updatelog/dev1.md](../updatelog/dev1.md) for the full implementation log.

### Role in One Sentence
Build the foundation everything else sits on: the SQLite connection wrapper,
all three database tables, and all model structs with their CRUD functions.

### Files Owned
| File | Status |
|------|--------|
| `include/database/db.h` + `src/database/db.cpp` | ✅ Done |
| `include/database/schema.h` + `src/database/schema.cpp` | ✅ Done |
| `include/models/user.h` + `src/models/user.cpp` | ✅ Done |
| `include/models/account.h` + `src/models/account.cpp` | ✅ Done |
| `include/models/transaction.h` + `src/models/transaction.cpp` | ✅ Done |
| `include/auth/password.h` + `src/auth/password.cpp` | ✅ Stub done |

### What Was Delivered
- `DB` class — wraps sqlite3 with `execute()` and `query()` using a binder-lambda pattern
- `Schema::init(db)` — creates all 3 tables (`users`, `accounts`, `transactions`) and seeds default admin
- `UserModel::` findById, findByUsername, create, findAll
- `AccountModel::` create (generates `ACC000001`…), findByUserId, findByAccountNumber, updateBalance, findAll
- `TransactionModel::` create, findPending, findByAccountId, updateStatus
- `Password::hash()` + `Password::verify()` — FNV-1a stub, used to hash the seeded admin password

---

## Dev 2 — Auth System

### Role in One Sentence
Implement register, login, password hashing, session tracking, and the
top-level `main.cpp` loop that routes users to the right menu after login.

### Files Owned
| File | What goes in it |
|------|----------------|
| `src/auth/password.cpp` | Hash + verify (minimal stub already exists — review it) |
| `include/auth/session.h` + `src/auth/session.cpp` | In-memory singleton: who is logged in right now |
| `include/auth/auth.h` + `src/auth/auth.cpp` | `registerUser()` and `loginUser()` |
| `src/main.cpp` | Top-level loop: show menu → login/register → route to user or admin menu |

---

### Step-by-Step: What To Implement

#### Step 1 — Review `password.cpp` (do this first)
Open `src/auth/password.cpp`. Dev 1 wrote a minimal FNV-1a hash to make the
schema seed compile. Read it. You have two choices:
- **Keep it as-is** — it works, it's consistent, the admin seed already used it.
- **Replace it** — you may use a stronger algorithm, BUT you must then also
  delete `banking.db` and let it regenerate on next run so the admin seed
  re-hashes with your new algorithm.

**Never change the algorithm without deleting the old `banking.db`.**
If the algorithm changes but the stored hash doesn't, `admin123` will stop working.

#### Step 2 — Implement `session.cpp`
The session is just a static variable that holds who is currently logged in.
There is no file I/O, no database, no network — just an in-memory struct.

```cpp
// What it should look like inside session.cpp:
static bool        active = false;
static SessionData current_session;

void Session::start(const SessionData& data) { current_session = data; active = true; }
void Session::end()                          { active = false; }
bool Session::isLoggedIn()                   { return active; }
SessionData Session::current() {
    if (!active) throw std::runtime_error("No active session");
    return current_session;
}
```

`SessionData` must hold at minimum:
- `int user_id` — the logged-in user's database id
- `std::string username`
- `std::string role` — either `"user"` or `"admin"`
- `int account_id` — the user's account id (set to `0` for admins, who have no account)

#### Step 3 — Implement `auth.cpp`

**`registerUser(db, name, username, password)`**
1. Call `UserModel::findByUsername(db, username)` — if it returns a value,
   the username is taken. Print an error and return `false`.
2. Call `Password::hash(password)` to get the hashed password.
3. Call `UserModel::create(db, name, username, hash, "user")` to create the user row.
4. Call `AccountModel::create(db, user.id)` to create their bank account.
5. Print: `"Registration successful! Your account number is: ACC000XXX"`
6. Return `true`.

**`loginUser(db, username, password)`**
1. Call `UserModel::findByUsername(db, username)` — if nullopt, return `false`.
2. Call `Password::verify(password, user.password_hash)` — if false, return `false`.
3. Build `SessionData`:
   - For role `"admin"`: `{ user.id, user.username, "admin", account_id = 0 }`
   - For role `"user"`: fetch the account with `AccountModel::findByUserId(db, user.id)`,
     then `{ user.id, user.username, "user", account.id }`
4. Call `Session::start(data)`.
5. Return `true`.

#### Step 4 — Implement `main.cpp`
Replace the current smoke-test with the real entry-point loop:

```
Open DB  →  Schema::init(db)  →  loop:

  === Banking Management System ===
  1. Login
  2. Register
  3. Exit
  > _

  On Login:
    prompt username, prompt password
    Auth::loginUser(db, username, password)
    → false: print "Invalid username or password." — loop again
    → true:  Session::current().role == "admin"  → AdminMenu::run(db)
             Session::current().role == "user"   → UserMenu::run(db)
    After the menu returns (user logged out), loop back to main screen.

  On Register:
    prompt name, username, password
    Auth::registerUser(db, name, username, password)
    loop back

  On Exit:
    print "Goodbye." — return 0
```

---

### How To Use Dev 1's Code

```cpp
#include "database/db.h"
#include "database/schema.h"
#include "models/user.h"
#include "models/account.h"
#include "auth/password.h"

DB db("banking.db");
Schema::init(db);

// Find a user
auto user = UserModel::findByUsername(db, "alice");
if (!user) { /* not found */ }

// Create a user
auto newUser = UserModel::create(db, "Alice", "alice", Password::hash("pass"), "user");

// Create their account
auto account = AccountModel::create(db, newUser.id);
```

---

### What To Remember

- **Never store a plain-text password.** Always call `Password::hash()` before
  passing to `UserModel::create()`.
- **Session is global state.** Only one user can be logged in at a time. Always
  call `Session::end()` when the user logs out before returning from the menu.
- **`main.cpp` includes both menus.** It is the only file that should include
  both `user/user_menu.h` and `admin/admin_menu.h`. No other file should do this.
- **Input validation.** In `registerUser`, reject empty username or password.
  In `loginUser`, trim leading/trailing spaces before checking — users often
  accidentally type a space.
- **Do not hardcode role checks.** Read the role from `Session::current().role`
  — never assume the logged-in user is an admin just because the code path
  came from the admin menu.
- **Password must echo as `*`.** When prompting for a password, consider hiding
  the input. If not implemented, at minimum note it as a known limitation.

---

### Contract You Must Deliver to Dev 3 & Dev 4

```cpp
// Dev 3 and Dev 4 call this to know who is logged in:
#include "auth/session.h"
SessionData s = Session::current();
s.user_id      // int
s.username     // std::string
s.role         // "user" or "admin"
s.account_id   // int (0 for admin)
```

If you change the field names in `SessionData`, Dev 3 and Dev 4 break.
Agree on these names before Dev 3 or Dev 4 write a single line.

---

## Dev 3 — User Features

### Role in One Sentence
Implement everything a logged-in regular user can do: view their balance,
view their transaction history, and send money to another account.

### Files Owned
| File | What goes in it |
|------|----------------|
| `include/user/user_menu.h` + `src/user/user_menu.cpp` | Console menu loop for regular users |
| `include/user/transfer.h` + `src/user/transfer.cpp` | Send money flow |
| `include/user/user_account.h` + `src/user/user_account.cpp` | View balance + history |

---

### Step-by-Step: What To Implement

#### Step 1 — Implement `user_account.cpp` (start here — no dependencies on transfer)

`UserAccount::showBalanceAndHistory(db)`:
1. Get `account_id` from `Session::current().account_id`.
2. Fetch the account: `AccountModel::findByUserId(db, session.user_id)`.
3. Print account number and current balance clearly:
   ```
   ╔══════════════════════════════╗
   Account   : ACC000002
   Balance   : $1,250.00
   ╚══════════════════════════════╝
   ```
4. Fetch transactions: `TransactionModel::findByAccountId(db, account.id)`.
5. If empty, print `"No transactions yet."` and return.
6. Print a table with columns: `ID | Date | From | To | Amount | Status`
7. On each row, show `(you)` next to the user's own account number so they
   can easily tell which direction money moved.

#### Step 2 — Implement `transfer.cpp`

`Transfer::sendMoney(db)`:
1. Get the sender's account via `AccountModel::findByUserId(db, session.user_id)`.
2. Prompt: `"Enter recipient account number: "` (e.g. `ACC000005`)
3. Look up recipient: `AccountModel::findByAccountNumber(db, input)`.
4. **Validate — stop and print an error for each of these:**
   - Recipient account does not exist → `"Account not found."`
   - Recipient is the same as sender → `"Cannot send money to yourself."`
   - Prompt: `"Enter amount: "` — if `amount <= 0` → `"Amount must be greater than zero."`
   - `sender.balance < amount` → `"Insufficient funds. Your balance is $X.XX"`
5. Prompt: `"Enter a note (optional — press Enter to skip): "`
6. Create the transaction:
   `TransactionModel::create(db, sender.id, recipient.id, amount, note)`
7. Print:
   ```
   Transfer submitted.
   Transaction #7 is pending admin approval.
   ```

**Important:** Do NOT update any account balances here. Balances only change
when an admin approves the transaction. Dev 4 handles that.

#### Step 3 — Implement `user_menu.cpp`

`UserMenu::run(db)`:
```
=== Welcome, Alice ===

1. View Balance & Transaction History
2. Send Money
3. Logout

> _
```
- On `1`: call `UserAccount::showBalanceAndHistory(db)`
- On `2`: call `Transfer::sendMoney(db)`
- On `3`: call `Session::end()`, print `"Logged out."`, `return` (do not loop again)
- On anything else: print `"Invalid option. Try again."`
- Loop until logout.

---

### How To Use Dev 1 & Dev 2's Code

```cpp
#include "auth/session.h"
#include "models/account.h"
#include "models/transaction.h"

// Get logged-in user info
SessionData s = Session::current();
int user_id    = s.user_id;
int account_id = s.account_id;

// Get their account
auto acc = AccountModel::findByUserId(db, user_id);

// Get their transactions
auto txns = TransactionModel::findByAccountId(db, acc->id);

// Create a new transfer
auto txn = TransactionModel::create(db, from_id, to_id, amount, note);
```

---

### What To Remember

- **Never touch account balances in transfer.cpp.** Your job is only to create
  a `PENDING` transaction. Balance changes happen in Dev 4's `approval.cpp`.
  If you update balances here, money will be double-spent when the admin approves.
- **Always re-fetch the account before showing balance.** Do not cache the
  balance — it may have changed since login if another transaction was approved.
- **Check that the recipient account actually exists** before showing the amount
  prompt. Don't ask for the amount if the recipient doesn't exist.
- **The note field is optional.** If the user presses Enter with no input,
  pass an empty string `""` to `TransactionModel::create()` — that is fine.
- **Show `(you)` in the history table.** When listing transactions, the user
  sees both `from_account` and `to_account` columns. Mark their own account
  number with `(you)` so it's clear whether they sent or received money.
- **Pending transactions still show in history.** Do not filter them out.
  The user should see the full picture including money they've sent but
  that hasn't been approved yet.
- **`UserMenu::run()` must call `Session::end()` before returning.** If it
  doesn't, the next person who sits at the terminal is still logged in as
  the previous user.

---

### Contract You Must Deliver to `main.cpp` (Dev 2)

```cpp
// Dev 2 calls this after a successful user login:
#include "user/user_menu.h"
UserMenu::run(db);
// When this returns, the user has logged out.
```

---

## Dev 4 — Admin Panel ✅ Complete

> See [updatelog/dev4.md](../updatelog/dev4.md) for the full implementation log.

### Role in One Sentence
Implement everything an admin can do: list pending transactions, approve or
reject them (updating balances atomically on approve), and view all user accounts.

### Files Owned
| File | What goes in it |
|------|----------------|
| `include/admin/admin_menu.h` + `src/admin/admin_menu.cpp` | ✅ Console menu loop for admin |
| `include/admin/approval.h` + `src/admin/approval.cpp` | ✅ List, approve, reject transactions |
| `include/admin/user_mgmt.h` + `src/admin/user_mgmt.cpp` | ✅ View all users and balances |

---

### Step-by-Step: What To Implement

#### Step 1 — Implement `user_mgmt.cpp` (start here — simplest, no tricky logic)

`UserMgmt::listAllUsers(db)`:
1. Fetch all users: `UserModel::findAll(db)`
2. For each user, fetch their account: `AccountModel::findByUserId(db, user.id)`
3. Print a table:
   ```
   ID  Name              Username    Account No.   Balance      Role    Joined
   ---  ----------------  ----------  ------------  -----------  ------  -------------------
   1   Administrator     admin       —             —            admin   2024-01-01 00:00:00
   2   Alice Johnson     alice       ACC000002     $1,250.00    user    2024-05-01 10:23:00
   ```
4. For admin users, show `—` in the Account No. and Balance columns
   (admins have no bank account).

#### Step 2 — Implement `approval.cpp`

**`Approval::listPending(db)`**
1. Fetch: `TransactionModel::findPending(db)`
2. If empty: print `"No pending transactions."` and return.
3. Print a table:
   ```
   ID   Date                  From Acc      To Acc        Amount     Note
   ----  --------------------  ------------  ------------  ---------  ----------
   3    2024-05-01 10:23:00   ACC000002     ACC000005     $500.00    rent
   ```

**`Approval::reviewTransaction(db)`**
This is the most important and most complex function in the whole project.
Follow these steps exactly:

1. Call `listPending(db)` first so the admin sees what's available.
2. Prompt: `"Enter Transaction ID to review (0 to cancel): "`
3. If input is `0`, return.
4. Fetch the transaction by id:
   ```cpp
   // There's no findById on TransactionModel yet — query directly via DB or
   // filter from findPending. Simplest: fetch pending list, find the one with matching id.
   ```
5. If not found or not `pending`, print `"Transaction not found or already reviewed."` — return.
6. Display full transaction details:
   ```
   Transaction #3
   From    : ACC000002 (Alice Johnson)
   To      : ACC000005 (Bob Smith)
   Amount  : $500.00
   Note    : rent
   Date    : 2024-05-01 10:23:00
   ```
7. Prompt: `"Action: (A)pprove  (R)eject  (C)ancel: "`

**On Approve:**
```
a) Re-fetch sender account (balance may have changed since the list was shown)
b) If sender.balance < amount → print "Insufficient funds. Cannot approve." → return
c) Begin a database transaction:
       db.execute("BEGIN;")
d) Deduct from sender:
       AccountModel::updateBalance(db, sender.id, sender.balance - amount)
e) Credit receiver:
       AccountModel::updateBalance(db, receiver.id, receiver.balance + amount)
f) Mark approved:
       TransactionModel::updateStatus(db, txn_id, "approved", Session::current().user_id)
g) Commit:
       db.execute("COMMIT;")
h) Print: "Transaction #3 approved. $500.00 transferred."
```

**On Reject:**
```
a) TransactionModel::updateStatus(db, txn_id, "rejected", Session::current().user_id)
b) Print: "Transaction #3 rejected."
```

**On Cancel:** just return.

#### Step 3 — Implement `admin_menu.cpp`

`AdminMenu::run(db)`:
```
=== Admin Panel — Welcome, Administrator ===

1. View Pending Transactions
2. Approve / Reject a Transaction
3. View All Users & Accounts
4. Logout

> _
```
- On `1`: call `Approval::listPending(db)`
- On `2`: call `Approval::reviewTransaction(db)`
- On `3`: call `UserMgmt::listAllUsers(db)`
- On `4`: `Session::end()`, print `"Logged out."`, `return`
- On anything else: `"Invalid option. Try again."`
- Loop until logout.

---

### How To Use Dev 1 & Dev 2's Code

```cpp
#include "auth/session.h"
#include "models/user.h"
#include "models/account.h"
#include "models/transaction.h"

// Get admin info
SessionData s     = Session::current();
int admin_user_id = s.user_id;

// List all pending transactions
auto pending = TransactionModel::findPending(db);

// Approve: update both balances then mark approved
db.execute("BEGIN;");
AccountModel::updateBalance(db, sender_id, new_sender_balance);
AccountModel::updateBalance(db, receiver_id, new_receiver_balance);
TransactionModel::updateStatus(db, txn_id, "approved", admin_user_id);
db.execute("COMMIT;");
```

---

### What To Remember

- **The `BEGIN` / `COMMIT` block on approve is not optional.**
  If your program crashes between updating the sender's balance and updating
  the receiver's balance, money disappears. Wrapping both updates in a
  transaction makes them atomic — either both happen or neither does.
  This is the most critical correctness rule in the entire project.

- **Always re-fetch the sender's balance right before approving.**
  The admin might approve a transaction that was valid when submitted but the
  sender's balance has since dropped (another transaction was also approved).
  Re-fetching prevents approving a transfer the sender can no longer cover.

- **`updateStatus` records who reviewed it.**
  Pass `Session::current().user_id` as the `reviewed_by` argument every time.
  This creates an audit trail — the database records which admin approved or
  rejected every transaction and at what time.

- **Admins have no account.** `Session::current().account_id` will be `0` for
  admins. Never try to look up an admin's account balance — they don't have one.

- **`listPending` is called inside `reviewTransaction`.**
  Always show the list before asking for a transaction ID, so the admin can
  see what IDs are available. Do not make them guess.

- **Rejecting does not touch balances.** When you reject, the only thing that
  changes is the transaction's `status` column. Do not deduct or refund anything.
  The money never moved in the first place (balances only change on approve).

- **`AdminMenu::run()` must call `Session::end()` before returning.**
  Same rule as Dev 3 — always clean up the session on logout.

---

### Contract You Must Deliver to `main.cpp` (Dev 2)

```cpp
// Dev 2 calls this after a successful admin login:
#include "admin/admin_menu.h"
AdminMenu::run(db);
// When this returns, the admin has logged out.
```

---

## Shared Rules For All Devs

### Input / Output
- Use `std::cout` for all output. No `printf`.
- Use `std::cin` for all input. After every `std::cin >>`, call
  `std::cin.ignore()` to clear the newline from the buffer before the next
  `std::getline`. Forgetting this causes the next input to be skipped.
- Always validate user input. If a menu expects `1`, `2`, or `3` and the user
  types `abc`, print an error and re-prompt — do not crash.

### Error Handling
- Wrap your `main.cpp` entry point in a `try / catch (std::exception&)` block.
  DB errors throw `std::runtime_error` — if uncaught the program just dies with
  no message. Dev 2 already has this in the smoke-test `main.cpp`.
- Inside menus, do not let a bad DB query crash the whole program. Catch errors,
  print a friendly message, and return to the menu.

### Headers and Includes
- Only include what you directly use. Do not include a header just because
  another file includes it.
- Never include `sqlite3.h` in your files. Use `DB` class from `database/db.h`
  instead. Only `db.cpp` and `schema.cpp` touch sqlite3 internals.

### Session Discipline
- Only Dev 2's `auth.cpp` calls `Session::start()`.
- Every menu (`UserMenu::run`, `AdminMenu::run`) must call `Session::end()` on logout.
- Never call `Session::current()` without first checking `Session::isLoggedIn()`
  if there's any chance the session might not be active — it throws.
