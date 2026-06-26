#pragma once
#include "database/db.h"

namespace UserMenu {
    // Runs the interactive console menu for a logged-in regular user.
    // Loops until the user selects Logout, at which point it calls Session::end() and returns.
    // Dev 2's main.cpp calls this after a successful user login.
    void run(DB& db);
}
