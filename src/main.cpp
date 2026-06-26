// =============================================================================
// OWNER : Dev 2 — Auth System
// FILE  : src/main.cpp
// ABOUT : Top-level entry point. Shows login/register menu and routes to
//         user or admin menu based on Session::current().role.
// =============================================================================
#include <iostream>
#include <string>
#include "database/db.h"
#include "database/schema.h"
#include "auth/auth.h"
#include "auth/session.h"
#include "user/user_menu.h"
#include "admin/admin_menu.h"

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

int main() {
    try {
        DB db("banking.db");
        Schema::init(db);

        while (true) {
            std::cout << "\n=== Banking Management System ===\n";
            std::cout << "1. Login\n";
            std::cout << "2. Register\n";
            std::cout << "3. Exit\n";
            std::cout << "> ";

            std::string choice;
            std::getline(std::cin, choice);
            choice = trim(choice);

            if (choice == "1") {
                std::cout << "Username: ";
                std::string username;
                std::getline(std::cin, username);
                username = trim(username);

                std::cout << "Password: ";
                std::string password;
                std::getline(std::cin, password);
                password = trim(password);

                if (Auth::loginUser(db, username, password)) {
                    SessionData session = Session::current();
                    if (session.role == "admin") {
                        AdminMenu::run(db);
                    } else {
                        UserMenu::run(db);
                    }
                } else {
                    std::cout << "Invalid username or password.\n";
                }

            } else if (choice == "2") {
                std::cout << "Name: ";
                std::string name;
                std::getline(std::cin, name);
                name = trim(name);

                std::cout << "Username: ";
                std::string username;
                std::getline(std::cin, username);
                username = trim(username);

                std::cout << "Password: ";
                std::string password;
                std::getline(std::cin, password);
                password = trim(password);

                Auth::registerUser(db, name, username, password);


            } else if (choice == "3") {
                std::cout << "Goodbye.\n";
                return 0;
            } else {
                std::cout << "Invalid option. Try again.\n";
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
