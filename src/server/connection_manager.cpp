#include "server/connection_manager.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <iostream>

namespace server {

ConnectionManager::ConnectionManager(std::shared_ptr<SessionManager> sm) : sessionManager(sm) {}

void ConnectionManager::add_client(int user_id, ClientHandler* client) {
    active_clients_[user_id] = client;
}

void ConnectionManager::remove_client(int user_id) {
    active_clients_.erase(user_id);
}

ClientHandler* ConnectionManager::get_client(int user_id) {
    auto it = active_clients_.find(user_id);
    if (it != active_clients_.end()) {
        return it->second;
    }
    return nullptr;
}

void ConnectionManager::sendToUser(int userId, const protocol::Message& msg) {
    if (!sessionManager) return;

    std::vector<int> fds = sessionManager->get_fds_by_user_id(userId);
    std::vector<uint8_t> data = msg.serialize();

    for (int fd : fds) {
        ssize_t sent = send(fd, data.data(), data.size(), 0);
        if (sent < 0) {
            if (logger::serverLogger) {
                logger::serverLogger->error("Failed to send push notification to fd=" + std::to_string(fd));
            }
        } else {
            if (logger::serverLogger) {
                logger::serverLogger->debug("Sent push notification to fd=" + std::to_string(fd));
            }
        }
    }
}

} // namespace server
