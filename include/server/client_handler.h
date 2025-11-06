#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/database_utils.h"
#include <vector>

namespace server {

class ClientHandler {
private:
    SessionManager& sessionManager;
    UserDatabase& userDatabase;

    // Handle specific message types
    void handleLoginRequest(int clientFd, const protocol::Message& msg);
    void handleLogoutRequest(int clientFd, const protocol::Message& msg);
    void handleHeartbeat(int clientFd, const protocol::Message& msg);
    void handleDisconnectRequest(int clientFd);

    // Send message to client
    bool sendMessage(int clientFd, const protocol::Message& msg);
    
    // Receive message from client
    protocol::Message receiveMessage(int clientFd);

public:
    ClientHandler(SessionManager& sm, UserDatabase& db);

    // Process incoming message from client
    void processMessage(int clientFd, const std::vector<uint8_t>& data);

    // Handle client disconnect
    void handleClientDisconnect(int clientFd);
};

} // namespace server

#endif // CLIENT_HANDLER_H
