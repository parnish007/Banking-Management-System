// =============================================================================
// OWNER : Dev 2 — Auth System
// FILE  : src/auth/auth.cpp
// ABOUT : Implement Auth::registerUser() and Auth::loginUser().
//
// TASKS:
//   [ ] registerUser(db, name, username, password)
//         → check username not already taken (UserModel::findByUsername)
//         → hash the password (Password::hash)
//         → create user with role "user" (UserModel::create)
//         → create account for that user (AccountModel::create)
//         → print "Registration successful. Account: ACC000XXX"
//         → return false if username already exists, true on success
//
//   [ ] loginUser(db, username, password)
//         → fetch user by username (UserModel::findByUsername)
//         → verify password (Password::verify)
//         → if admin: SessionData{id, username, "admin", account_id=0}
//         → if user:  fetch account, SessionData{id, username, "user", account_id}
//         → call Session::start()
//         → return false on bad credentials, true on success
// =============================================================================
#include "auth/auth.h"
#include "database/db.h"
#include "models/user.h"
#include "models/account.h"
#include "auth/password.h"
#include "auth/session.h"
#include <iostream>

bool Auth::registerUser(DB& db,
                        const std::string& name,
                        const std::string& username,
                        const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cout << "Username and password cannot be empty.\n";
        return false;
    }

    auto existing = UserModel::findByUsername(db, username);
    if (existing) {
        std::cout << "Username already taken.\n";
        return false;
    }

    std::string hashed = Password::hash(password);
    auto user = UserModel::create(db, name, username, hashed, "user");
    auto account = AccountModel::create(db, user.id);

    std::cout << "Registration successful! Your account number is: " << account.account_number << "\n";
    return true;
}

bool Auth::loginUser(DB& db,
                     const std::string& username,
                     const std::string& password) {
    auto user = UserModel::findByUsername(db, username);
    if (!user) {
        return false;
    }

    if (!Password::verify(password, user->password_hash)) {
        return false;
    }

    SessionData session_data;
    session_data.user_id = user->id;
    session_data.username = user->username;
    session_data.role = user->role;

    if (user->role == "admin") {
        session_data.account_id = 0;
    } else {
        auto account = AccountModel::findByUserId(db, user->id);
        if (!account) {
            std::cerr << "Error: user has no account.\n";
            return false;
        }
        session_data.account_id = account->id;
    }

    Session::start(session_data);
    return true;
}
