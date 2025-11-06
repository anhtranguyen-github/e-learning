#ifndef CLIENT_UI_H
#define CLIENT_UI_H

#include "client/network.h"
#include <string>

namespace client {

class UI {
private:
    NetworkClient& network;
    bool running;

    void displayMainMenu();
    void displayLoggedInMenu();
    void handleLogin();
    void handleLogout();
    void handleViewLessons();
    void handleStudyLesson();
    void handleDoExercise();
    void handleSubmitAnswer();
    void handleViewResults();
    void clearScreen();
    
    // Helper to parse and display lesson list
    void parseLessonList(const std::string& payload);
    void displayLessonContent(const std::string& payload);

public:
    UI(NetworkClient& net);

    void run();
    void stop();
};

} // namespace client

#endif // CLIENT_UI_H
