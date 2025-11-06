#include "server/user_manager.h"
#include "common/logger.h"
#include <cstring>

namespace server {

UserManager::UserManager(std::shared_ptr<Database> database) : db(database) {
}

bool UserManager::verifyCredentials(const std::string& username, const std::string& password) {
    if (!db || !db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Database not connected");
        }
        return false;
    }
    
    const char* paramValues[2] = {username.c_str(), password.c_str()};
    
    PGresult* res = db->execParams(
        "SELECT user_id FROM users WHERE username = $1 AND password_hash = $2",
        2, paramValues
    );
    
    if (!res) {
        return false;
    }
    
    int nRows = PQntuples(res);
    PQclear(res);
    
    bool verified = (nRows > 0);
    
    if (logger::serverLogger) {
        if (verified) {
            logger::serverLogger->info("User '" + username + "' authenticated successfully");
        } else {
            logger::serverLogger->warn("Authentication failed for user '" + username + "'");
        }
    }
    
    return verified;
}

bool UserManager::addUser(const std::string& username, const std::string& password,
                          const std::string& role, const std::string& level) {
    if (!db || !db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Database not connected");
        }
        return false;
    }
    
    // Check if user already exists
    if (userExists(username)) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("User '" + username + "' already exists");
        }
        return false;
    }
    
    const char* paramValues[4] = {
        username.c_str(),
        password.c_str(),
        role.c_str(),
        level.c_str()
    };
    
    PGresult* res = db->execParams(
        "INSERT INTO users (username, password_hash, role, level) VALUES ($1, $2, $3, $4)",
        4, paramValues
    );
    
    if (!res) {
        return false;
    }
    
    PQclear(res);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("User '" + username + "' added successfully");
    }
    
    return true;
}

bool UserManager::userExists(const std::string& username) {
    if (!db || !db->isConnected()) {
        return false;
    }
    
    const char* paramValues[1] = {username.c_str()};
    
    PGresult* res = db->execParams(
        "SELECT user_id FROM users WHERE username = $1",
        1, paramValues
    );
    
    if (!res) {
        return false;
    }
    
    int nRows = PQntuples(res);
    PQclear(res);
    
    return (nRows > 0);
}

int UserManager::getUserId(const std::string& username) {
    if (!db || !db->isConnected()) {
        return -1;
    }
    
    const char* paramValues[1] = {username.c_str()};
    
    PGresult* res = db->execParams(
        "SELECT user_id FROM users WHERE username = $1",
        1, paramValues
    );
    
    if (!res || PQntuples(res) == 0) {
        if (res) PQclear(res);
        return -1;
    }
    
    int userId = std::stoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    
    return userId;
}

std::string UserManager::getUserRole(const std::string& username) {
    if (!db || !db->isConnected()) {
        return "";
    }
    
    const char* paramValues[1] = {username.c_str()};
    
    PGresult* res = db->execParams(
        "SELECT role FROM users WHERE username = $1",
        1, paramValues
    );
    
    if (!res || PQntuples(res) == 0) {
        if (res) PQclear(res);
        return "";
    }
    
    std::string role = PQgetvalue(res, 0, 0);
    PQclear(res);
    
    return role;
}

std::string UserManager::getUserLevel(const std::string& username) {
    if (!db || !db->isConnected()) {
        return "";
    }
    
    const char* paramValues[1] = {username.c_str()};
    
    PGresult* res = db->execParams(
        "SELECT level FROM users WHERE username = $1",
        1, paramValues
    );
    
    if (!res || PQntuples(res) == 0) {
        if (res) PQclear(res);
        return "";
    }
    
    std::string level = PQgetvalue(res, 0, 0);
    PQclear(res);
    
    return level;
}

bool UserManager::updateUserLevel(const std::string& username, const std::string& level) {
    if (!db || !db->isConnected()) {
        return false;
    }
    
    const char* paramValues[2] = {level.c_str(), username.c_str()};
    
    PGresult* res = db->execParams(
        "UPDATE users SET level = $1 WHERE username = $2",
        2, paramValues
    );
    
    if (!res) {
        return false;
    }
    
    PQclear(res);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Updated level for user '" + username + "' to '" + level + "'");
    }
    
    return true;
}

bool UserManager::updatePassword(const std::string& username, const std::string& newPassword) {
    if (!db || !db->isConnected()) {
        return false;
    }
    
    const char* paramValues[2] = {newPassword.c_str(), username.c_str()};
    
    PGresult* res = db->execParams(
        "UPDATE users SET password_hash = $1 WHERE username = $2",
        2, paramValues
    );
    
    if (!res) {
        return false;
    }
    
    PQclear(res);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Password updated for user '" + username + "'");
    }
    
    return true;
}

} // namespace server
