#include "server/user_manager.h"
#include "server/database.h"
#include "common/logger.h"
#include <cstring>

namespace server {

UserManager::UserManager(Database& db) : db_(db) {}

bool UserManager::verify_credentials(const std::string& username, const std::string& password) {
    // This method should ideally hash the password and compare it with the stored hash.
    // For simplicity, we are currently storing plain text passwords.
    std::string query = "SELECT password_hash FROM users WHERE username = $1";
    const char* values[] = {username.c_str()};
    PGresult* res = db_.execParams(query, 1, values);

    if (res && PQntuples(res) == 1) {
        std::string stored_password = PQgetvalue(res, 0, 0);
        PQclear(res);
        return stored_password == password;
    }

    if (res) {
        PQclear(res);
    }
    return false;
}

int UserManager::get_user_id(const std::string& username) {
    std::string query = "SELECT user_id FROM users WHERE username = $1";
    const char* values[] = {username.c_str()};
    PGresult* res = db_.execParams(query, 1, values);

    if (res && PQntuples(res) == 1) {
        int user_id = std::stoi(PQgetvalue(res, 0, 0));
        PQclear(res);
        return user_id;
    }

    if (res) {
        PQclear(res);
    }
    return -1;
}

void UserManager::add_client(int user_id, ClientHandler* client) {
    active_clients_[user_id] = client;
}

void UserManager::remove_client(int user_id) {
    active_clients_.erase(user_id);
}

ClientHandler* UserManager::get_client(int user_id) {
    auto it = active_clients_.find(user_id);
    if (it != active_clients_.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace server
