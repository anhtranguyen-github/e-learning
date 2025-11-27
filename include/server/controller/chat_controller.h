#ifndef CHAT_CONTROLLER_H
#define CHAT_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/connection_manager.h"
#include "server/database.h"
#include <memory>
#include <string>

namespace server {

class ChatController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ConnectionManager> connectionManager;
    std::shared_ptr<Database> db;

    void sendMessage(int clientFd, const protocol::Message& msg);

public:
    ChatController(std::shared_ptr<SessionManager> sessionMgr, 
                std::shared_ptr<ConnectionManager> connMgr,
                std::shared_ptr<Database> database);

    void handle_private_message(int clientFd, const protocol::Message& msg);
    void handle_chat_history(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // CHAT_CONTROLLER_H
