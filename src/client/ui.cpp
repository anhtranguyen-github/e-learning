#include "client/ui.h"
#include "common/logger.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace client {

UI::UI(NetworkClient& net) : network(net), running(false) {
}

void UI::clearScreen() {
    std::cout << "\033[2J\033[1;1H"; // ANSI escape codes to clear screen
}

void UI::displayMainMenu() {
    clearScreen();
    std::cout << "========================================\n";
    std::cout << "     TCP Learning App - Client\n";
    std::cout << "========================================\n\n";
    
    if (!network.isConnected()) {
        std::cout << "Status: Not Connected\n\n";
        std::cout << "1. Connect to Server\n";
        std::cout << "2. Exit\n\n";
    } else if (!network.isLoggedIn()) {
        std::cout << "Status: Connected (Not Logged In)\n\n";
        std::cout << "1. Login\n";
        std::cout << "2. Disconnect\n";
        std::cout << "3. Exit\n\n";
    } else {
        displayLoggedInMenu();
        return;
    }
    
    std::cout << "Choose an option: ";
}

void UI::displayLoggedInMenu() {
    clearScreen();
    std::cout << "========================================\n";
    std::cout << "     TCP Learning App - Client\n";
    std::cout << "========================================\n\n";
    std::cout << "Status: Logged In\n";
    std::cout << "Session Token: " << network.getSessionToken().substr(0, 8) << "...\n\n";
    std::cout << "1. Logout\n";
    std::cout << "2. View Status\n";
    std::cout << "3. Exit\n\n";
    std::cout << "Choose an option: ";
}

void UI::handleLogin() {
    std::string username, password;
    
    std::cout << "\n--- Login ---\n";
    std::cout << "Username: ";
    std::cin >> username;
    std::cout << "Password: ";
    std::cin >> password;
    
    std::cout << "\nAttempting to login...\n";
    
    if (network.login(username, password)) {
        std::cout << "Login successful!\n";
        std::cout << "Press Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    } else {
        std::cout << "Login failed. Please check your credentials.\n";
        std::cout << "Press Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
}

void UI::handleLogout() {
    std::cout << "\nLogging out...\n";
    
    if (network.logout()) {
        std::cout << "Logout successful!\n";
    } else {
        std::cout << "Logout failed.\n";
    }
    
    std::cout << "Press Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void UI::run() {
    running = true;
    
    // Start heartbeat thread
    std::thread heartbeatThread([this]() {
        while (running) {
            if (network.shouldSendHeartbeat()) {
                network.sendHeartbeat();
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    while (running) {
        displayMainMenu();
        
        int choice;
        std::cin >> choice;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (!network.isConnected()) {
            // Not connected menu
            switch (choice) {
                case 1:
                    std::cout << "\nConnecting to server...\n";
                    if (network.connect()) {
                        std::cout << "Connected successfully!\n";
                    } else {
                        std::cout << "Connection failed.\n";
                    }
                    std::cout << "Press Enter to continue...";
                    std::cin.ignore();
                    std::cin.get();
                    break;
                
                case 2:
                    running = false;
                    break;
                
                default:
                    std::cout << "Invalid option.\n";
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    break;
            }
        } else if (!network.isLoggedIn()) {
            // Connected but not logged in menu
            switch (choice) {
                case 1:
                    handleLogin();
                    break;
                
                case 2:
                    network.disconnect();
                    std::cout << "\nDisconnected from server.\n";
                    std::cout << "Press Enter to continue...";
                    std::cin.ignore();
                    std::cin.get();
                    break;
                
                case 3:
                    running = false;
                    break;
                
                default:
                    std::cout << "Invalid option.\n";
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    break;
            }
        } else {
            // Logged in menu
            switch (choice) {
                case 1:
                    handleLogout();
                    break;
                
                case 2:
                    std::cout << "\n--- Status ---\n";
                    std::cout << "Connected: Yes\n";
                    std::cout << "Logged In: Yes\n";
                    std::cout << "Session Token: " << network.getSessionToken() << "\n";
                    std::cout << "Press Enter to continue...";
                    std::cin.ignore();
                    std::cin.get();
                    break;
                
                case 3:
                    running = false;
                    break;
                
                default:
                    std::cout << "Invalid option.\n";
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    break;
            }
        }
    }

    // Wait for heartbeat thread to finish
    heartbeatThread.join();
    
    // Cleanup
    if (network.isConnected()) {
        network.disconnect();
    }
}

void UI::stop() {
    running = false;
}

} // namespace client
