#include "server/repository/user_repository.h"
#include "common/logger.h"

namespace server {

UserRepository::UserRepository(std::shared_ptr<Database> database) : db(database) {}

bool UserRepository::verifyCredentials(const std::string& username, const std::string& password) {
    if (!db) return false;

    // This method should ideally hash the password and compare it with the stored hash.
    // For simplicity, we are currently storing plain text passwords.
    std::string query = "SELECT password_hash FROM users WHERE username = $1";
    const char* values[] = {username.c_str()};
    PGresult* res = db->execParams(query, 1, values);

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

int UserRepository::getUserId(const std::string& username) {
    if (!db) return -1;

    std::string query = "SELECT user_id FROM users WHERE username = $1";
    const char* values[] = {username.c_str()};
    PGresult* res = db->execParams(query, 1, values);

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

User UserRepository::findById(int id) {
    // Placeholder implementation if needed later
    return User();
}

} // namespace server
