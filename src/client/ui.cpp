#include "client/ui.h"
#include "common/logger.h"
#include "common/payloads.h"
#include "common/utils.h"
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
        if (network.getUserRole() == "admin") {
            displayAdminMenu();
        } else {
            displayLoggedInMenu();
        }
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
    std::cout << "3. Do Exercise\n";
    std::cout << "4. Submit Answer\n";
    std::cout << "5. View Results\n";
    std::cout << "6. View Exercises\n";
    std::cout << "7. View Exams\n";
    std::cout << "8. Chat\n";
    std::cout << "9. View Status\n";
    std::cout << "10. Logout\n";
    std::cout << "11. Exit\n\n";
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
            if (network.getUserRole() == "admin") {
                // Admin Menu
                switch (choice) {
                    case 1:
                        handleAdminGameMenu();
                        break;
                    case 2:
                        handleViewLessons();
                        break;
                    case 3:
                        handleViewExercises();
                        break;
                    case 4:
                        handleViewExams();
                        break;
                    case 5:
                        handleViewResults();
                        break;
                    case 6:
                        handleChat();
                        break;
                    case 7:
                        std::cout << "\n--- Status ---\n";
                        std::cout << "Connected: Yes\n";
                        std::cout << "Logged In: Yes (Admin)\n";
                        std::cout << "Session Token: " << network.getSessionToken() << "\n";
                        std::cout << "Press Enter to continue...";
                        std::cin.ignore();
                        std::cin.get();
                        break;
                    case 8:
                        handleLogout();
                        break;
                    case 9:
                        running = false;
                        break;
                    default:
                        std::cout << "Invalid option.\n";
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        break;
                }
            } else {
                // Logged in menu (Student/Teacher)
                switch (choice) {
                    case 1:
                        handleViewLessons();
                        break;
                    
                    case 2:
                        handleStudyLesson();
                        break;

                    case 3:
                        handleDoExercise();
                        break;

                    case 4:
                        handleSubmitAnswer();
                        break;

                    case 5:
                        handleViewResults();
                        break;

                    case 6:
                        handleViewExercises();
                        break;

                    case 7:
                        handleViewExams();
                        break;
                    
                    case 8:
                        handleChat();
                        break;

                    case 9:
                        std::cout << "\n--- Status ---\n";
                        std::cout << "Connected: Yes\n";
                        std::cout << "Logged In: Yes\n";
                        std::cout << "Session Token: " << network.getSessionToken() << "\n";
                        std::cout << "Press Enter to continue...";
                        std::cin.ignore();
                        std::cin.get();
                        break;
                    
                    case 10:
                        handleLogout();
                        break;
                    
                    case 11:
                        running = false;
                        break;
                    
                    default:
                        std::cout << "Invalid option.\n";
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        break;
                }
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
    
    auto parts = utils::split(payload, ';');
    if (parts.empty()) {
        std::cout << "No lessons found.\n";
        return;
    }
    
    int count = 0;
    try {
        count = std::stoi(parts[0]);
    } catch (...) {
        std::cout << "Invalid response format.\n";
        return;
    }
    
    std::cout << "Total lessons: " << count << "\n\n";
    
    if (count == 0) {
        std::cout << "No lessons match your filters.\n";
        return;
    }
    
    for (size_t i = 1; i < parts.size(); ++i) {
        Payloads::LessonMetadataDTO dto;
        dto.deserialize(parts[i]);
        
        std::cout << "[" << dto.id << "] " << dto.title << "\n";
        std::cout << "    Topic: " << dto.topic << " | Level: " << dto.level << "\n\n";
    }
}

void UI::displayLessonContent(const std::string& payload) {
    Payloads::LessonDTO dto;
    dto.deserialize(payload);
    
    std::cout << "Lesson: " << dto.title << " (ID: " << dto.id << ")\n";
    std::cout << "Topic: " << dto.topic << " | Level: " << dto.level << "\n";
    std::cout << "----------------------------------------\n";
    
    if (!dto.videoUrl.empty()) std::cout << "VIDEO: " << dto.videoUrl << "\n";
    if (!dto.audioUrl.empty()) std::cout << "AUDIO: " << dto.audioUrl << "\n";
    if (!dto.textContent.empty()) std::cout << "TEXT:\n" << dto.textContent << "\n";
    
    if (!dto.vocabulary.empty()) {
        std::cout << "\nVOCABULARY:\n";
        for (const auto& item : dto.vocabulary) {
            std::cout << "  - " << item << "\n";
        }
    }
    
    if (!dto.grammar.empty()) {
        std::cout << "\nGRAMMAR:\n";
        for (const auto& item : dto.grammar) {
            std::cout << "  - " << item << "\n";
        }
    }
    
    std::cout << "----------------------------------------\n";
}

void UI::handleDoExercise() {
    int exerciseId;
    int exerciseTypeChoice;
    protocol::MsgCode exerciseType;

    std::cout << "\n--- Do Exercise ---\n";
    std::cout << "Enter exercise ID: ";
    std::cin >> exerciseId;

    std::cout << "Choose exercise type:\n";
    std::cout << "1. Multiple Choice\n";
    std::cout << "2. Fill In the Blank\n";
    std::cout << "3. Order Sentence\n";
    std::cout << "4. Rewrite Sentence\n";
    std::cout << "5. Write Paragraph\n";
    std::cout << "6. Speaking Topic\n";
    std::cout << "Type: ";
    std::cin >> exerciseTypeChoice;

    switch (exerciseTypeChoice) {
        case 1: exerciseType = protocol::MsgCode::MULTIPLE_CHOICE_REQUEST; break;
        case 2: exerciseType = protocol::MsgCode::FILL_IN_REQUEST; break;
        case 3: exerciseType = protocol::MsgCode::SENTENCE_ORDER_REQUEST; break;
        case 4: exerciseType = protocol::MsgCode::REWRITE_SENTENCE_REQUEST; break;
        case 5: exerciseType = protocol::MsgCode::WRITE_PARAGRAPH_REQUEST; break;
        case 6: exerciseType = protocol::MsgCode::SPEAKING_TOPIC_REQUEST; break;
        default: std::cout << "Invalid choice\n"; return;
    }

    if (!network.requestExercise(exerciseType, exerciseId)) {
        std::cout << "Failed to send request.\n";
        return;
    }

    protocol::Message response = network.receiveMessage();
    std::cout << "\nExercise Content:\n" << response.toString() << "\n";
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void UI::handleSubmitAnswer() {
    int targetId;
    std::string targetType, answer;

    std::cout << "\n--- Submit Answer ---\n";
    std::cout << "Enter target type (exam/exercise): ";
    std::cin >> targetType;
    std::cout << "Enter target ID: ";
    std::cin >> targetId;
    std::cout << "Enter your answer: ";
    std::cin.ignore();
    std::getline(std::cin, answer);

    if (!network.submitAnswer(targetType, targetId, answer)) {
        std::cout << "Failed to send request.\n";
        return;
    }

    protocol::Message response = network.receiveMessage();
    std::cout << "\nServer response:\n" << response.toString() << "\n";
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void UI::handleViewResults() {
    std::cout << "\n--- View Results ---\n";
    if (!network.requestResultList()) {
        std::cout << "Failed to send request.\n";
        return;
    }

    protocol::Message response = network.receiveMessage();
    std::cout << "\nYour Results:\n" << response.toString() << "\n";
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void UI::handleViewExercises() {
    std::cout << "\n--- View Exercises ---\n";
    if (!network.requestExercises()) {
        std::cout << "Failed to send request.\n";
        return;
    }

    protocol::Message response = network.receiveMessage();
    if (response.code == protocol::MsgCode::EXERCISE_LIST_SUCCESS) {
        std::cout << "\n=== Exercise List ===\n\n";
        parseExerciseList(response.toString());
    } else {
        std::cout << "\nFailed to get exercise list: " << response.toString() << "\n";
    }
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void UI::handleViewExams() {
    std::cout << "\n--- View Exams ---\n";
    if (!network.requestExams()) {
        std::cout << "Failed to send request.\n";
        return;
    }

    protocol::Message response = network.receiveMessage();
    if (response.code == protocol::MsgCode::EXAM_LIST_SUCCESS) {
        std::cout << "\n=== Exam List ===\n\n";
        parseExamList(response.toString());
    } else {
        std::cout << "\nFailed to get exam list: " << response.toString() << "\n";
    }
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void UI::handleChat() {
    bool chatRunning = true;
    while (chatRunning) {
        clearScreen();
        std::cout << "\n--- Chat Menu ---\n";
        std::cout << "1. Send Private Message\n";
        std::cout << "2. View Chat History\n";
        std::cout << "3. Back to Main Menu\n\n";
        std::cout << "Choose an option: ";

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1:
                handleSendPrivateMessage();
                break;
            case 2:
                handleViewChatHistory();
                break;
            case 3:
                chatRunning = false;
                break;
            default:
                std::cout << "Invalid option.\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                break;
        }
    }
}

void UI::handleSendPrivateMessage() {
    std::string recipient, message;

    std::cout << "\n--- Send Private Message ---\n";
    std::cout << "Recipient: ";
    std::cin >> recipient;
    std::cout << "Message: ";
    std::cin.ignore();
    std::getline(std::cin, message);

    if (!network.sendPrivateMessage(recipient, message)) {
        std::cout << "Failed to send request.\n";
        return;
    }

    protocol::Message response = network.receiveMessage();
    std::cout << "\nServer response:\n" << response.toString() << "\n";
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void UI::handleViewChatHistory() {
    std::string otherUser;

    std::cout << "\n--- View Chat History ---\n";
    std::cout << "Enter username to view history with: ";
    std::cin >> otherUser;

    if (!network.requestChatHistory(otherUser)) {
        std::cout << "Failed to send request.\n";
        return;
    }

    protocol::Message response = network.receiveMessage();
    std::cout << "\nChat History:\n" << response.toString() << "\n";
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void UI::parseExerciseList(const std::string& payload) {
    // Format: <count>;<id1>|<lesson_id1>|<title1>|<type1>|<level1>;...
    std::stringstream ss(payload);
    std::string segment;

    // Get count
    if (!std::getline(ss, segment, ';')) {
        std::cout << "No exercises found.\n";
        return;
    }
    int count = 0;
    try {
        count = std::stoi(segment);
    } catch (const std::invalid_argument& e) {
        std::cout << "Invalid exercise list format (count).\n";
        return;
    }

    std::cout << "Total exercises: " << count << "\n\n";
    if (count == 0) {
        return;
    }

    // Get exercises
    while(std::getline(ss, segment, ';')) {
        std::stringstream meta_ss(segment);
        std::string id, lesson_id, title, type, level;

        std::getline(meta_ss, id, '|');
        std::getline(meta_ss, lesson_id, '|');
        std::getline(meta_ss, title, '|');
        std::getline(meta_ss, type, '|');
        std::getline(meta_ss, level, '|');

        std::cout << "[" << id << "] " << title << "\n";
        std::cout << "    Lesson ID: " << lesson_id << " | Type: " << type << " | Level: " << level << "\n\n";
    }
}

void UI::parseExamList(const std::string& payload) {
    // Format: <count>;<id1>|<lesson_id1>|<title1>|<type1>|<level1>;...
    std::stringstream ss(payload);
    std::string segment;

    // Get count
    if (!std::getline(ss, segment, ';')) {
        std::cout << "No exams found.\n";
        return;
    }
    int count = 0;
    try {
        count = std::stoi(segment);
    } catch (const std::invalid_argument& e) {
        std::cout << "Invalid exam list format (count).\n";
        return;
    }

    std::cout << "Total exams: " << count << "\n\n";
    if (count == 0) {
        return;
    }

    // Get exams
    while(std::getline(ss, segment, ';')) {
        std::stringstream meta_ss(segment);
        std::string id, lesson_id, title, type, level;

        std::getline(meta_ss, id, '|');
        std::getline(meta_ss, lesson_id, '|');
        std::getline(meta_ss, title, '|');
        std::getline(meta_ss, type, '|');
        std::getline(meta_ss, level, '|');

        std::cout << "[" << id << "] " << title << "\n";
        std::cout << "    Lesson ID: " << lesson_id << " | Type: " << type << " | Level: " << level << "\n\n";
    }
}


void UI::displayAdminMenu() {
    clearScreen();
    std::cout << "========================================\n";
    std::cout << "     TCP Learning App - Admin Panel\n";
    std::cout << "========================================\n\n";
    std::cout << "Status: Logged In (Admin)\n";
    std::cout << "Session Token: " << network.getSessionToken().substr(0, 8) << "...\n\n";
    std::cout << "1. Manage Games\n";
    std::cout << "2. View Lesson List\n";
    std::cout << "3. View Exercises\n";
    std::cout << "4. View Exams\n";
    std::cout << "5. View Results\n";
    std::cout << "6. Chat\n";
    std::cout << "7. View Status\n";
    std::cout << "8. Logout\n";
    std::cout << "9. Exit\n\n";
    std::cout << "Choose an option: ";
}

void UI::handleAdminGameMenu() {
    bool inGameMenu = true;
    while (inGameMenu) {
        clearScreen();
        std::cout << "\n--- Game Management ---\n";
        std::cout << "1. Create Game\n";
        std::cout << "2. Update Game\n";
        std::cout << "3. Delete Game\n";
        std::cout << "4. Back to Admin Menu\n\n";
        std::cout << "Choose an option: ";

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1:
                handleAdminCreateGame();
                break;
            case 2:
                handleAdminUpdateGame();
                break;
            case 3:
                handleAdminDeleteGame();
                break;
            case 4:
                inGameMenu = false;
                break;
            default:
                std::cout << "Invalid option.\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                break;
        }
    }
}

void UI::handleAdminCreateGame() {
    std::string type, level, json;
    
    std::cout << "\n--- Create Game ---\n";
    std::cout << "Type (sentence_match/word_match/image_match): ";
    std::cin >> type;
    std::cout << "Level (beginner/intermediate/advanced): ";
    std::cin >> level;
    std::cout << "Question JSON: ";
    std::cin.ignore();
    std::getline(std::cin, json);

    if (network.requestCreateGame(type, level, json)) {
        protocol::Message response = network.receiveMessage();
        std::cout << "Response: " << response.toString() << "\n";
    } else {
        std::cout << "Failed to send request.\n";
    }
    
    std::cout << "Press Enter to continue...";
    std::cin.get(); 
}

void UI::handleAdminUpdateGame() {
    std::string id, type, level, json;
    
    std::cout << "\n--- Update Game ---\n";
    std::cout << "Game ID: ";
    std::cin >> id;
    std::cout << "Type (sentence_match/word_match/image_match): ";
    std::cin >> type;
    std::cout << "Level (beginner/intermediate/advanced): ";
    std::cin >> level;
    std::cout << "Question JSON: ";
    std::cin.ignore();
    std::getline(std::cin, json);

    if (network.requestUpdateGame(id, type, level, json)) {
        protocol::Message response = network.receiveMessage();
        std::cout << "Response: " << response.toString() << "\n";
    } else {
        std::cout << "Failed to send request.\n";
    }

    std::cout << "Press Enter to continue...";
    std::cin.get();
}

void UI::handleAdminDeleteGame() {
    std::string id;
    
    std::cout << "\n--- Delete Game ---\n";
    std::cout << "Game ID: ";
    std::cin >> id;

    if (network.requestDeleteGame(id)) {
        protocol::Message response = network.receiveMessage();
        std::cout << "Response: " << response.toString() << "\n";
    } else {
        std::cout << "Failed to send request.\n";
    }

    std::cout << "Press Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

} // namespace client
