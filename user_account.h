#pragma once
#include "database/db.h"

namespace UserAccount {
    // Displays the logged-in user's current balance and full transaction history.
    // Reads session from Session::current() — must be called while a user is logged in.
    void showBalanceAndHistory(DB& db);
}
