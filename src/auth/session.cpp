// =============================================================================
// OWNER : Dev 2 — Auth System
// FILE  : src/auth/session.cpp
// ABOUT : Implement the Session namespace declared in include/auth/session.h.
//         This is a simple in-memory singleton — just a static variable.
//
// TASKS:
//   [ ] start(data)     → store data in a static SessionData variable,
//                          set a static bool isActive = true
//   [ ] end()           → clear the static variable, set isActive = false
//   [ ] isLoggedIn()    → return isActive
//   [ ] current()       → if isActive return the stored SessionData,
//                          else throw std::runtime_error("No active session")
// =============================================================================
#include "auth/session.h"

static bool active = false;
static SessionData current_session;

void Session::start(const SessionData& data) {
    current_session = data;
    active = true;
}

void Session::end() {
    active = false;
}

bool Session::isLoggedIn() {
    return active;
}

SessionData Session::current() {
    if (!active) {
        throw std::runtime_error("No active session");
    }
    return current_session;
}
