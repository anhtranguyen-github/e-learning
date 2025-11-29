#ifndef SERVER_REPOSITORY_CHAT_REPOSITORY_H
#define SERVER_REPOSITORY_CHAT_REPOSITORY_H

#include "server/database.h"
#include "server/model/chat_message.h"
#include <vector>
#include <memory>
#include <optional>

namespace server {

class ChatRepository {
private:
    std::shared_ptr<Database> db;

public:
    explicit ChatRepository(std::shared_ptr<Database> db);

    // Save a new message
    int saveMessage(const ChatMessage& message);

    // Get chat history between two users
    std::vector<ChatMessage> getChatHistory(int userId1, int userId2, int limit = 50, int offset = 0);

    // Get list of recent chats (latest message per user)
    // Returns a list of ChatMessage where each message represents the last interaction with a unique user
    std::vector<ChatMessage> getRecentChats(int userId);

    // Mark messages as read
    void markMessagesAsRead(int senderId, int receiverId);
};

} // namespace server

#endif // SERVER_REPOSITORY_CHAT_REPOSITORY_H
