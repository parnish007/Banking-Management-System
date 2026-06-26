// =============================================================================
// OWNER : Dev 2 — Auth System
// FILE  : include/auth/auth.h
// ABOUT : High-level register and login entry-points used by main.cpp.
//         Internally calls Password, UserModel, AccountModel, and Session.
//
// MUST PROVIDE:
//   namespace Auth
//     - bool registerUser(DB& db,
//                         const std::string& name,
//                         const std::string& username,
//                         const std::string& password)
//         → creates a User row + Account row, returns false if username taken
//
//     - bool loginUser(DB& db,
//                      const std::string& username,
//                      const std::string& password)
//         → validates credentials, starts Session, returns false on bad creds
//
// DEPENDENCIES: db.h, models/user.h, models/account.h, auth/password.h, auth/session.h
// =============================================================================
#pragma once

#include <string>

class DB;

namespace Auth {
    bool registerUser(DB& db,
                      const std::string& name,
                      const std::string& username,
                      const std::string& password);

    bool loginUser(DB& db,
                   const std::string& username,
                   const std::string& password);
}
