#ifndef SERVER_CONTROLLER_CHAT_CONTROLLER_H
#define SERVER_CONTROLLER_CHAT_CONTROLLER_H

#include "server/repository/chat_repository.h"
#include "server/connection_manager.h"
#include "server/session.h"
#include "server/repository/user_repository.h"
#include "common/payloads.h"
#include "common/protocol.h"
#include <memory>

namespace server {

class ChatController {
private:
    std::shared_ptr<ChatRepository> chatRepository;
    std::shared_ptr<UserRepository> userRepository;
    std::shared_ptr<ConnectionManager> connectionManager;
    std::shared_ptr<SessionManager> sessionManager;

public:
    ChatController(std::shared_ptr<ChatRepository> chatRepo,
                   std::shared_ptr<UserRepository> userRepo,
                   std::shared_ptr<ConnectionManager> connMgr,
                   std::shared_ptr<SessionManager> sessionMgr);

    // Handle SEND_CHAT_PRIVATE_REQUEST
    void handleSendPrivateMessage(int clientFd, const protocol::Message& msg);

    // Handle CHAT_HISTORY_REQUEST
    void handleGetChatHistory(int clientFd, const protocol::Message& msg);

    // Handle RECENT_CHATS_REQUEST
    void handleGetRecentChats(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // SERVER_CONTROLLER_CHAT_CONTROLLER_H
