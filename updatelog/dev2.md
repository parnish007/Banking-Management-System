# Dev 2 — Update Log
**Role:** Auth System — Registration, Login, Session Management, Main Entry Point
**Date:** 2026-06-02
**Status:** Complete

---

## What Was Implemented

### 1. `SessionData` Struct + `Session` Namespace — In-Memory Session State
**Files:** `include/auth/session.h` · `src/auth/session.cpp`

A global in-memory singleton that tracks who is currently logged in. No database
reads, no file I/O — purely a static variable that any module can read.

**`SessionData` struct:**
| Field | Type | Purpose |
|-------|------|---------|
| `user_id` | `int` | Primary key of the logged-in user in the `users` table |
| `username` | `std::string` | Display name, used for welcome messages |
| `role` | `std::string` | `"user"` or `"admin"` — controls which menu is shown |
| `account_id` | `int` | Primary key in `accounts` table. `0` for admins (no account) |

**`Session` namespace functions:**
| Function | Purpose |
|----------|---------|
| `start(data)` | Stores `SessionData`, sets `active = true` — called on successful login |
| `end()` | Sets `active = false` — must be called by every menu on logout |
| `isLoggedIn()` | Returns the `active` flag — safe pre-check before calling `current()` |
| `current()` | Returns the stored `SessionData`. Throws `std::runtime_error` if not active |

**Design decision — why a global static instead of passing session everywhere?**
Without a global session, every function that needs to know who is logged in would
require an extra `const SessionData&` parameter — `UserMenu::run(db, session)`,
`Transfer::sendMoney(db, session)`, `Approval::reviewTransaction(db, session)`.
This creates noise across every module. A read-only global session is the standard
pattern for single-user console applications where only one person is active at a time.

**Design decision — `current()` returns by value, not reference:**
Returning a copy of `current_session` means the caller's copy is safe even if
`Session::end()` is called between `current()` and the caller using the data.
A reference to the static variable would become dangling after `end()` resets the state.

---

### 2. `Auth` Namespace — Register and Login
**Files:** `include/auth/auth.h` · `src/auth/auth.cpp`

Two functions that form the public interface between the UI (`main.cpp`) and the
model layer (Dev 1's `UserModel`, `AccountModel`, `Password`). Neither function
touches raw SQL — they only call Dev 1's model functions.

| Function | Signature | Returns |
|----------|-----------|---------|
| `registerUser` | `(DB&, name, username, password)` | `true` on success, `false` on failure |
| `loginUser` | `(DB&, username, password)` | `true` on success, `false` on bad credentials |

**`registerUser` internal flow:**
1. Reject if `username` or `password` is empty — prints error, returns `false`
2. Check username availability via `UserModel::findByUsername` — prints "taken" if exists
3. Hash password with `Password::hash()` — plain text is never stored
4. Create user row: `UserModel::create(db, name, username, hash, "user")`
5. Create account row: `AccountModel::create(db, user.id)` — generates `ACC000XXX`
6. Print confirmation with account number, return `true`

**`loginUser` internal flow:**
1. Fetch user by username — return `false` silently if not found
2. Verify password with `Password::verify(input, stored_hash)` — return `false` on mismatch
3. Build `SessionData`: admin gets `account_id = 0`, user gets real account id
4. Guard against `nullopt` before dereferencing the account optional
5. Call `Session::start(session_data)`, return `true`

**Design decision — silent failure on bad username:**
Not distinguishing "user not found" from "wrong password" prevents an attacker
from enumerating valid usernames. Both cases print the same generic message
in `main.cpp`: `"Invalid username or password."`

**Design decision — forward declaration `class DB;` in header:**
`auth.h` uses `class DB;` instead of `#include "database/db.h"`. This is valid
because the parameter is `DB&` (a reference — the compiler only needs the name,
not the full layout). It avoids pulling `sqlite3.h` into every file that includes
`auth.h`, keeping compile times down.

---

### 3. `main.cpp` — Application Entry Point
**File:** `src/main.cpp`

Replaced Dev 1's smoke test with the real application loop. Opens the database,
runs schema init, then loops on a 3-option menu until the user exits.

**Loop behaviour:**
| Choice | Action |
|--------|--------|
| `1` Login | Prompt username + password → `Auth::loginUser()` → route by role |
| `2` Register | Prompt name + username + password → `Auth::registerUser()` |
| `3` Exit | Print `"Goodbye."` → `return 0` |
| anything else | Print `"Invalid option. Try again."` → loop |

**`trim()` helper — strips whitespace from every input:**
Removes leading/trailing spaces, tabs, `\r`, and `\n`. Critical on Windows where
`getline` leaves `\r` at the end of lines from `\r\n` line endings — without trimming,
`choice == "1"` would silently fail because the actual string is `"1\r"`.

**Design decision — `std::getline` for all input:**
`std::cin >>` stops at whitespace and leaves the newline in the input buffer. The
next `getline` call then reads an empty string (the leftover `\n`) instead of the
user's actual input. Using `getline` consistently for every prompt eliminates this
class of bug entirely.

**Design decision — `main.cpp` is the only file that includes both menus:**
Only `main.cpp` includes both `user/user_menu.h` and `admin/admin_menu.h`. The
routing logic ("admin goes here, user goes there") lives in one place. No other
module needs to know that both menus exist.

---

### 4. Menu Stubs
**Files:** `include/user/user_menu.h` · `src/user/user_menu.cpp`
          `include/admin/admin_menu.h` · `src/admin/admin_menu.cpp`

Added function declarations and empty stub implementations so `main.cpp` compiles
and links. Without a concrete symbol, the linker throws
`"undefined reference to UserMenu::run(DB&)"`.

Both stubs already include `auth/session.h` so Dev 3 and Dev 4 can call
`Session::current()` without adding any extra includes.

---

## Libraries Used

### Standard Library Headers

| Header | File | Why |
|--------|------|-----|
| `<string>` | All files | `std::string` for all text fields and inputs |
| `<stdexcept>` | `session.h`, `session.cpp` | `std::runtime_error` thrown by `Session::current()` |
| `<iostream>` | `auth.cpp`, `main.cpp` | `std::cout` for normal output, `std::cerr` for errors |

**Why `std::getline` and not `std::cin >>`:**
`cin >>` reads one whitespace-delimited token and leaves the newline in the buffer.
Every subsequent `getline` call reads an empty string until the buffer is manually
cleared with `cin.ignore()`. `getline` reads the full line including spaces and
discards the delimiter — no buffer poisoning, no phantom empty lines.

**Why `std::cerr` for the account-not-found error:**
`cerr` writes to `stderr`, which is unbuffered and separate from `stdout`. Error
conditions should go to `cerr` so they can be distinguished when output is
redirected in scripts or tests.

---

## What Dev 1 Was Used

```cpp
#include "database/db.h"    // DB::execute(), DB::query(), DB::lastInsertId()
#include "models/user.h"    // UserModel::findByUsername(), UserModel::create()
#include "models/account.h" // AccountModel::findByUserId(), AccountModel::create()
#include "auth/password.h"  // Password::hash(), Password::verify()
```

`Password::hash()` and `Password::verify()` were written by Dev 1 as a minimal
FNV-1a stub needed for schema seeding. Reviewed and kept as-is — the admin seed
in `banking.db` was generated with this algorithm. Changing it requires deleting
`banking.db` so it regenerates on next run.

---

## What Dev 3 & Dev 4 Get From Dev 2

```cpp
#include "auth/session.h"

// Available after any successful login:
SessionData s = Session::current();
s.user_id     // int    — users.id of logged-in user
s.username    // string — display name
s.role        // string — "user" or "admin"
s.account_id  // int    — accounts.id (0 for admin)

// MUST call at the end of every menu run() before returning:
Session::end();
```

**Rule:** Every implementation of `UserMenu::run(DB&)` and `AdminMenu::run(DB&)`
MUST call `Session::end()` before the function returns. If it doesn't, the next
person at the terminal inherits the previous session.

---

## Verified Output (2026-06-02)

Build: zero errors, zero warnings on GCC 15.2 / MinGW.

```
=== Register new user ===
> Name: Username: Password:
Registration successful! Your account number is: ACC000002    ✅

=== Register duplicate username ===
> Name: Username: Password:
Username already taken.                                        ✅

=== Login wrong password ===
> Username: Password:
Invalid username or password.                                  ✅

=== Login as user (alice) ===
> Username: Password:
[UserMenu::run called — stub returns, back to main menu]       ✅

=== Login as admin ===
> Username: Password:
[AdminMenu::run called — stub returns, back to main menu]      ✅
```
