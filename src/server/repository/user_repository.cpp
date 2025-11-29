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
    if (!db) return User();

    std::string query = "SELECT user_id, username, password_hash, full_name, role, level FROM users WHERE user_id = $1";
    std::string idStr = std::to_string(id);
    const char* values[] = {idStr.c_str()};
    PGresult* res = db->execParams(query, 1, values);

    if (res && PQntuples(res) == 1) {
        int userId = std::stoi(PQgetvalue(res, 0, 0));
        std::string username = PQgetvalue(res, 0, 1);
        std::string password = PQgetvalue(res, 0, 2);
        std::string fullName = PQgetvalue(res, 0, 3);
        std::string role = PQgetvalue(res, 0, 4);
        std::string level = PQgetvalue(res, 0, 5);
        
        PQclear(res);
        return User(userId, username, password, fullName, role, level);
    }

    if (res) {
        PQclear(res);
    }
    return User();
}

} // namespace server
