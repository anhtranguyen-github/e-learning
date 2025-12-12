#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <string>
#include <memory>
#include <unordered_map>

#include "common/protocol.h"
#include "server/session.h"
#include <vector>

namespace server {

class ClientHandler;

class ConnectionManager {
public:
    ConnectionManager(std::shared_ptr<SessionManager> sm);

    void add_client(int user_id, ClientHandler* client);
    void remove_client(int user_id);
    ClientHandler* get_client(int user_id);

    // Send message to a specific user (all active sessions)
    void sendToUser(int userId, const protocol::Message& msg);

    // Check if user is online
    bool isUserOnline(int userId) const;

private:
    std::unordered_map<int, ClientHandler*> active_clients_;
    std::shared_ptr<SessionManager> sessionManager;
};

} // namespace server

#endif // CONNECTION_MANAGER_H
