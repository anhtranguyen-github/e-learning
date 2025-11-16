#ifndef CHAT_HANDLER_H
#define CHAT_HANDLER_H

#include "common/protocol.h"

namespace server {

class ClientHandler;
class UserManager;
class Database;

class ChatHandler {
public:
    ChatHandler(UserManager& user_manager, Database& db);
    void handle_chat_message(ClientHandler* client, const protocol::Message& msg);

private:
    UserManager& user_manager_;
    Database& db_;

    void handle_private_message(ClientHandler* client, const protocol::Message& msg);
    void handle_chat_history(ClientHandler* client, const protocol::Message& msg);
};

} // namespace server

#endif // CHAT_HANDLER_H
