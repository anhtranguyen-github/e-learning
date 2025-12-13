#ifndef SERVER_CONTROLLER_CHAT_CONTROLLER_H
#define SERVER_CONTROLLER_CHAT_CONTROLLER_H

#include "server/repository/chat_repository.h"
#include "server/connection_manager.h"
#include "server/session.h"
#include "server/repository/user_repository.h"
#include "common/payloads.h"
#include "common/protocol.h"
#include <memory>
#include <chrono>
#include <map>
#include <set>
#include <string>

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
    void handleUserSendPrivateMessage(int clientFd, const protocol::Message& msg);

    // Handle CHAT_HISTORY_REQUEST
    void handleUserGetChatHistory(int clientFd, const protocol::Message& msg);

    // Handle RECENT_CHATS_REQUEST
    void handleUserGetRecentChats(int clientFd, const protocol::Message& msg);

    // Voice Call Handlers
    void handleCallInitiate(int clientFd, const protocol::Message& msg);
    void handleCallAnswer(int clientFd, const protocol::Message& msg);
    void handleCallDecline(int clientFd, const protocol::Message& msg);
    void handleCallEnd(int clientFd, const protocol::Message& msg);

    // Call Timeout Processing
    void processCallTimeouts();

private:
    struct PendingCall {
        std::string caller;
        std::string receiver;
        std::chrono::steady_clock::time_point startTime;
    };

    // key: receiver username (since one user can only receive one call at a time effectively)
    // or key: caller? Better key: receiver, to easily check if they are being called.
    // Actually, allowing key to be consistent. 
    // Let's store by receiver for timeout checking? No, simple vector or map is fine.
    // Map <Receiver, PendingCall> ensures one pending call per receiver.
    std::map<std::string, PendingCall> pendingCalls;

    // Set of users currently in a call (busy)
    std::set<std::string> activeUsersInCall;
};

} // namespace server

#endif // SERVER_CONTROLLER_CHAT_CONTROLLER_H
