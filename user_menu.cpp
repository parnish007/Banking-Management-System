#include "user/user_menu.h"
#include "user/user_account.h"
#include "user/transfer.h"
#include "auth/session.h"

#include <iostream>
#include <string>
#include <limits>

// ─────────────────────────────────────────────────────────────────────────────
void UserMenu::run(DB& db) {

    // Capture the username once at entry — safe because we hold the session.
    const std::string username = Session::current().username;

    while (true) {
        std::cout << "\n=== Welcome, " << username << " ===\n\n";
        std::cout << "1. View Balance & Transaction History\n";
        std::cout << "2. Send Money\n";
        std::cout << "3. Logout\n";
        std::cout << "\n> ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            // Non-integer input — clear the error state and try again
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid option. Try again.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                try {
                    UserAccount::showBalanceAndHistory(db);
                } catch (const std::exception& e) {
                    std::cout << "Error loading account data: " << e.what() << "\n";
                }
                break;

            case 2:
                try {
                    Transfer::sendMoney(db);
                } catch (const std::exception& e) {
                    std::cout << "Error processing transfer: " << e.what() << "\n";
                }
                break;

            case 3:
                // ── Logout ────────────────────────────────────────────────────
                Session::end();
                std::cout << "Logged out.\n";
                return;   // Returns control to main.cpp's login loop

            default:
                std::cout << "Invalid option. Try again.\n";
                break;
        }
    }
}
