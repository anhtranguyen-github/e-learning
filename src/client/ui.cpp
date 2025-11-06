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
    std::cout << "1. View Lesson List\n";
    std::cout << "2. Study Lesson\n";
    std::cout << "3. View Status\n";
    std::cout << "4. Logout\n";
    std::cout << "5. Exit\n\n";
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
                    handleViewLessons();
                    break;
                
                case 2:
                    handleStudyLesson();
                    break;
                
                case 3:
                    std::cout << "\n--- Status ---\n";
                    std::cout << "Connected: Yes\n";
                    std::cout << "Logged In: Yes\n";
                    std::cout << "Session Token: " << network.getSessionToken() << "\n";
                    std::cout << "Press Enter to continue...";
                    std::cin.ignore();
                    std::cin.get();
                    break;
                
                case 4:
                    handleLogout();
                    break;
                
                case 5:
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

void UI::handleViewLessons() {
    std::string topic, level;
    
    std::cout << "\n--- View Lesson List ---\n";
    std::cout << "Filter by topic (leave empty for all): ";
    std::cin.ignore();
    std::getline(std::cin, topic);
    
    std::cout << "Filter by level (beginner/intermediate/advanced, empty for all): ";
    std::getline(std::cin, level);
    
    std::cout << "\nRequesting lesson list...\n";
    
    if (!network.requestLessonList(topic, level)) {
        std::cout << "Failed to send request.\n";
        std::cout << "Press Enter to continue...";
        std::cin.get();
        return;
    }
    
    // Wait for response
    std::cout << "Waiting for response...\n";
    protocol::Message response = network.receiveMessage();
    
    if (response.code == protocol::MsgCode::LESSON_LIST_SUCCESS) {
        std::cout << "\n=== Lesson List ===\n\n";
        parseLessonList(response.toString());
    } else if (response.code == protocol::MsgCode::LESSON_LIST_FAILURE) {
        std::cout << "\nFailed to get lesson list: " << response.toString() << "\n";
    } else {
        std::cout << "\nUnexpected response code: " << static_cast<int>(response.code) << "\n";
    }
    
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void UI::handleStudyLesson() {
    int lessonId;
    std::string lessonType;
    
    std::cout << "\n--- Study Lesson ---\n";
    std::cout << "Enter lesson ID: ";
    std::cin >> lessonId;
    
    std::cout << "Choose content type:\n";
    std::cout << "  - video\n";
    std::cout << "  - audio\n";
    std::cout << "  - text\n";
    std::cout << "  - vocabulary\n";
    std::cout << "  - grammar\n";
    std::cout << "  - full (all content)\n";
    std::cout << "Type: ";
    std::cin >> lessonType;
    
    std::cout << "\nRequesting lesson content...\n";
    
    if (!network.requestStudyLesson(lessonId, lessonType)) {
        std::cout << "Failed to send request.\n";
        std::cout << "Press Enter to continue...";
        std::cin.ignore();
        std::cin.get();
        return;
    }
    
    // Wait for response
    std::cout << "Waiting for response...\n";
    protocol::Message response = network.receiveMessage();
    
    if (response.code == protocol::MsgCode::STUDY_LESSON_SUCCESS) {
        std::cout << "\n=== Lesson Content ===\n\n";
        displayLessonContent(response.toString());
    } else if (response.code == protocol::MsgCode::STUDY_LESSON_FAILURE) {
        std::cout << "\nFailed to get lesson: " << response.toString() << "\n";
    } else {
        std::cout << "\nUnexpected response code: " << static_cast<int>(response.code) << "\n";
    }
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void UI::parseLessonList(const std::string& payload) {
    // Format: <count>;<lesson1_meta>;<lesson2_meta>;...
    // Lesson metadata: <id>|<title>|<topic>|<level>
    
    size_t pos = 0;
    size_t nextPos = payload.find(';');
    
    if (nextPos == std::string::npos) {
        std::cout << "No lessons found.\n";
        return;
    }
    
    std::string countStr = payload.substr(0, nextPos);
    int count = std::stoi(countStr);
    
    std::cout << "Total lessons: " << count << "\n\n";
    
    if (count == 0) {
        std::cout << "No lessons match your filters.\n";
        return;
    }
    
    pos = nextPos + 1;
    
    for (int i = 0; i < count && pos < payload.length(); ++i) {
        nextPos = payload.find(';', pos);
        std::string lessonMeta;
        
        if (nextPos == std::string::npos) {
            lessonMeta = payload.substr(pos);
        } else {
            lessonMeta = payload.substr(pos, nextPos - pos);
        }
        
        // Parse lesson metadata: <id>|<title>|<topic>|<level>
        size_t p1 = lessonMeta.find('|');
        size_t p2 = lessonMeta.find('|', p1 + 1);
        size_t p3 = lessonMeta.find('|', p2 + 1);
        
        if (p1 != std::string::npos && p2 != std::string::npos && p3 != std::string::npos) {
            std::string id = lessonMeta.substr(0, p1);
            std::string title = lessonMeta.substr(p1 + 1, p2 - p1 - 1);
            std::string topic = lessonMeta.substr(p2 + 1, p3 - p2 - 1);
            std::string level = lessonMeta.substr(p3 + 1);
            
            std::cout << "[" << id << "] " << title << "\n";
            std::cout << "    Topic: " << topic << " | Level: " << level << "\n\n";
        }
        
        pos = (nextPos == std::string::npos) ? payload.length() : nextPos + 1;
    }
}

void UI::displayLessonContent(const std::string& payload) {
    // Format depends on lesson type:
    // VIDEO|<url>
    // AUDIO|<url>
    // TEXT|<content>
    // VOCABULARY|<word1>,<word2>,...
    // GRAMMAR|<rule1>,<rule2>,...
    // FULL|ID:<id>|TITLE:<title>|...
    
    size_t pipePos = payload.find('|');
    if (pipePos == std::string::npos) {
        std::cout << "Invalid lesson content format.\n";
        return;
    }
    
    std::string type = payload.substr(0, pipePos);
    std::string content = payload.substr(pipePos + 1);
    
    std::cout << "Content Type: " << type << "\n";
    std::cout << "----------------------------------------\n";
    
    if (type == "VIDEO" || type == "AUDIO") {
        std::cout << "URL: " << content << "\n";
    } else if (type == "TEXT") {
        std::cout << content << "\n";
    } else if (type == "VOCABULARY" || type == "GRAMMAR") {
        std::cout << "Items:\n";
        size_t pos = 0;
        while (pos < content.length()) {
            size_t commaPos = content.find(',', pos);
            std::string item;
            if (commaPos == std::string::npos) {
                item = content.substr(pos);
                pos = content.length();
            } else {
                item = content.substr(pos, commaPos - pos);
                pos = commaPos + 1;
            }
            if (!item.empty()) {
                std::cout << "  - " << item << "\n";
            }
        }
    } else if (type == "FULL") {
        // Parse full content with multiple fields
        std::cout << content << "\n";
    } else {
        std::cout << "Unknown type: " << content << "\n";
    }
    
    std::cout << "----------------------------------------\n";
}

} // namespace client
