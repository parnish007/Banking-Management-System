// =============================================================================
// OWNER : Dev 2 — Auth System
// FILE  : include/auth/session.h
// ABOUT : Global in-memory session state. Holds who is currently logged in.
//         Dev 3 and Dev 4 call Session::current() to get the active user.
//
// MUST PROVIDE:
//   struct SessionData
//     - int user_id
//     - std::string username
//     - std::string role   ("user" | "admin")
//     - int account_id     (0 for admin, actual id for users)
//
//   namespace Session
//     - void             start(const SessionData& data)   → log in
//     - void             end()                            → log out
//     - bool             isLoggedIn()
//     - SessionData      current()   → throws if not logged in
//
// DEPENDENCIES: none
// =============================================================================
#pragma once

#include <string>
#include <stdexcept>

struct SessionData {
    int user_id;
    std::string username;
    std::string role;
    int account_id;
};

namespace Session {
    void start(const SessionData& data);
    void end();
    bool isLoggedIn();
    SessionData current();
}
