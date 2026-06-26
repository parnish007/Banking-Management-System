# Banking Management System

A console-based banking system written in **C++17** with **SQLite3** as the data store.
Two roles: **User** (register, login, send money) and **Admin** (approve / reject transactions).

---

## Quick Navigation

| I want to… | Go to |
|------------|-------|
| Build and run the project | [Build Guide](#build--run) |
| Understand the full system design | [docs/architecture.md](docs/architecture.md) |
| See ERD, class diagrams, call chains | [docs/diagrams.md](docs/diagrams.md) |
| See the database tables (SQL) | [docs/database-schema.md](docs/database-schema.md) |
| Know what each dev owns | [docs/dev-assignments.md](docs/dev-assignments.md) |
| Read contribution rules | [docs/rules.md](docs/rules.md) |
| Understand the PR / branch workflow | [docs/git-workflow.md](docs/git-workflow.md) |
| See what Dev 1 implemented | [updatelog/dev1.md](updatelog/dev1.md) |
| See what Dev 2 implemented | [updatelog/dev2.md](updatelog/dev2.md) |
| Find a specific source file | [Repository Structure](#repository-structure) |

---

## Tech Stack

| Layer | Technology |
|-------|-----------|
| Language | C++17 |
| Database | SQLite 3.45.1 (bundled amalgamation — no install needed) |
| Build system | CMake 3.16+ with MinGW Makefiles |
| Compiler | GCC / g++ (MinGW) |

No external dependencies. Everything needed is inside the repo.

---

## Build & Run

**Requirements:** g++ (MinGW), CMake 3.16+, mingw32-make — all on PATH.

```bat
build.bat
```

Or manually:

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

The database file `banking.db` is created automatically on first run.

> Full details → [docs/build-guide.md](docs/build-guide.md)

---

## Default Credentials

| Username | Password | Role |
|----------|----------|------|
| `admin` | `admin123` | Admin |

New user accounts are created through the app's Register option.

---

## Current App State

The application boots, shows the main menu, and handles login and registration.
User and Admin menus are stub placeholders until Dev 3 and Dev 4 finish.

```
=== Banking Management System ===
1. Login
2. Register
3. Exit
> _
```

**Working now:**
- Register a new user account
- Login as user or admin (routes to the correct menu)
- Reject duplicate usernames
- Reject wrong passwords with a generic error message

**Coming next (Dev 3 & Dev 4):**
- Send money, view balance, transaction history (Dev 3)
- Approve / reject transactions, view all users (Dev 4)

---

## Features

### User
- Register a new account
- Login / Logout
- Send money to another account — creates a **pending** transaction *(Dev 3)*
- View current balance *(Dev 3)*
- View full transaction history *(Dev 3)*

### Admin
- Login / Logout
- View all pending transactions *(Dev 4)*
- Approve or reject a transaction — approve updates both balances atomically *(Dev 4)*
- View all users and their account balances *(Dev 4)*

### Transaction Lifecycle
```
User sends money → status: PENDING
       ↓
Admin logs in → reviews transaction
       ↓
 Approve → balances updated → status: APPROVED
 Reject  → no balance change → status: REJECTED
```

---

## Repository Structure

```
Banking-Management-System/
│
├── docs/                          # Project-wide documentation
│   ├── architecture.md            # System design and data flow diagrams
│   ├── diagrams.md                # ERD, class diagrams, call chain sequences
│   ├── database-schema.md         # SQL table definitions and relationships
│   ├── dev-assignments.md         # Who owns what, build order, dependencies
│   ├── build-guide.md             # Detailed build instructions
│   ├── git-workflow.md            # Branch strategy and PR process
│   └── rules.md                   # Contribution rules for all devs
│
├── updatelog/                     # Per-dev implementation logs
│   ├── dev1.md                    # Dev 1: DB layer + models (✅ complete)
│   └── dev2.md                    # Dev 2: Auth system + main loop (✅ complete)
│
├── include/                       # Header files (declarations only)
│   ├── database/
│   │   ├── db.h                   # [Dev 1 ✅] Core DB wrapper class
│   │   └── schema.h               # [Dev 1 ✅] Schema initialisation
│   ├── models/
│   │   ├── user.h                 # [Dev 1 ✅] User struct + CRUD
│   │   ├── account.h              # [Dev 1 ✅] Account struct + CRUD
│   │   └── transaction.h          # [Dev 1 ✅] Transaction struct + CRUD
│   ├── auth/
│   │   ├── auth.h                 # [Dev 2 ✅] Register + login entry-points
│   │   ├── session.h              # [Dev 2 ✅] SessionData struct + Session namespace
│   │   └── password.h             # [Dev 1 ✅] FNV-1a hash + verify
│   ├── user/
│   │   ├── user_menu.h            # [Dev 3 ⏳] User console menu
│   │   ├── transfer.h             # [Dev 3 ⏳] Send money flow
│   │   └── user_account.h         # [Dev 3 ⏳] View balance + history
│   └── admin/
│       ├── admin_menu.h           # [Dev 4 ⏳] Admin console menu
│       ├── approval.h             # [Dev 4 ⏳] Approve / reject transactions
│       └── user_mgmt.h            # [Dev 4 ⏳] View all users
│
├── src/                           # Implementation files
│   ├── main.cpp                   # [Dev 2 ✅] Login/register loop + role routing
│   ├── database/
│   │   ├── db.cpp                 # [Dev 1 ✅] complete
│   │   └── schema.cpp             # [Dev 1 ✅] complete
│   ├── models/
│   │   ├── user.cpp               # [Dev 1 ✅] complete
│   │   ├── account.cpp            # [Dev 1 ✅] complete
│   │   └── transaction.cpp        # [Dev 1 ✅] complete
│   ├── auth/
│   │   ├── auth.cpp               # [Dev 2 ✅] complete
│   │   ├── session.cpp            # [Dev 2 ✅] complete
│   │   └── password.cpp           # [Dev 1 ✅] complete
│   ├── user/
│   │   ├── user_menu.cpp          # [Dev 3 ⏳] stub — pending implementation
│   │   ├── transfer.cpp           # [Dev 3 ⏳] stub — pending implementation
│   │   └── user_account.cpp       # [Dev 3 ⏳] stub — pending implementation
│   └── admin/
│       ├── admin_menu.cpp         # [Dev 4 ⏳] stub — pending implementation
│       ├── approval.cpp           # [Dev 4 ⏳] stub — pending implementation
│       └── user_mgmt.cpp          # [Dev 4 ⏳] stub — pending implementation
│
├── lib/
│   └── sqlite3/                   # Bundled SQLite 3.45.1 amalgamation
│
├── .github/
│   ├── workflows/build.yml        # CI — builds on every push and PR
│   ├── CODEOWNERS                 # Auto-assigns supervisor review on every PR
│   └── PULL_REQUEST_TEMPLATE.md   # Checklist prefilled on every new PR
│
├── CMakeLists.txt                 # Build configuration
├── build.bat                      # One-click Windows build script
└── .gitignore
```

---

## Team & Build Order

Build order matters — each dev depends on the one before.

| Dev | Role | Status | Update Log |
|-----|------|--------|-----------|
| **Dev 1** | Database & Models | ✅ Complete | [updatelog/dev1.md](updatelog/dev1.md) |
| **Dev 2** | Auth System | ✅ Complete | [updatelog/dev2.md](updatelog/dev2.md) |
| **Dev 3** | User Features | 🔓 Unblocked — ready to start | — |
| **Dev 4** | Admin Panel | 🔓 Unblocked — ready to start | — |

Dev 3 and Dev 4 can work in parallel — both depend only on Dev 2 which is now merged.

> Full ownership breakdown → [docs/dev-assignments.md](docs/dev-assignments.md)

---

## Contribution Rules

Before pushing, every dev must:
1. Write `updatelog/devN.md` covering what was built, libraries used, and design reasoning
2. Ensure the build passes with zero errors
3. Use the commit message format: `Dev N [Role]: brief summary`
4. Open a PR — do not push directly to `main`

> Full rules → [docs/rules.md](docs/rules.md)
> Branch strategy → [docs/git-workflow.md](docs/git-workflow.md)
