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
    void clearScreen();

public:
    UI(NetworkClient& net);

    void run();
    void stop();
};

} // namespace client

#endif // CLIENT_UI_H
