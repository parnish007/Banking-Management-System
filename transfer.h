#pragma once
#include "database/db.h"

namespace Transfer {
    // Prompts the logged-in user for a recipient account number, amount, and optional note,
    // then creates a PENDING transaction. Does NOT update any balances — that is Dev 4's job.
    // Reads session from Session::current() — must be called while a user is logged in.
    void sendMoney(DB& db);
}
