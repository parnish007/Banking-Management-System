#include "user/user_menu.h"

#include "auth/session.h"
#include "user/transfer.h"
#include "user/user_account.h"

#include <iostream>
#include <limits>
#include <string>

namespace {

void clearLine() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace

void UserMenu::run(DB& db) {
    const std::string username = Session::current().username;

    while (true) {
        std::cout << "\n=== Welcome, " << username << " ===\n\n";
        std::cout << "1. View Balance & Transaction History\n";
        std::cout << "2. Send Money\n";
        std::cout << "3. Logout\n";
        std::cout << "\n> ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            clearLine();
            std::cout << "Invalid option. Try again.\n";
            continue;
        }
        clearLine();

        try {
            switch (choice) {
                case 1:
                    UserAccount::showBalanceAndHistory(db);
                    break;
                case 2:
                    Transfer::sendMoney(db);
                    break;
                case 3:
                    Session::end();
                    std::cout << "Logged out.\n";
                    return;
                default:
                    std::cout << "Invalid option. Try again.\n";
                    break;
            }
        } catch (const std::exception& e) {
            std::cout << "Operation failed: " << e.what() << "\n";
        }
    }
}
