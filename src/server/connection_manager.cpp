#include "server/connection_manager.h"

namespace server {

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

} // namespace server
