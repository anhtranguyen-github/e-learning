#include "server/database_utils.h"
#include "common/logger.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

namespace server {

UserDatabase::UserDatabase(const std::string& filePath) : dbFilePath(filePath) {
    // Create data directory if it doesn't exist
    mkdir("data", 0755);
    
    loadFromFile();
    
    // Add default users if database is empty
    if (users.empty()) {
        addUser("admin", "admin123");
        addUser("user1", "password1");
        addUser("test", "test");
        saveToFile();
        
        if (logger::serverLogger) {
            logger::serverLogger->info("Created default users");
        }
    }
}

void UserDatabase::loadFromFile() {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    std::ifstream file(dbFilePath);
    if (!file.is_open()) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("User database file not found, creating new one");
        }
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string username = line.substr(0, pos);
            std::string password = line.substr(pos + 1);
            users[username] = password;
        }
    }
    
    file.close();
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Loaded " + std::to_string(users.size()) + " users from database");
    }
}

void UserDatabase::saveToFile() {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    std::ofstream file(dbFilePath);
    if (!file.is_open()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to save user database");
        }
        return;
    }
    
    for (const auto& pair : users) {
        file << pair.first << ":" << pair.second << std::endl;
    }
    
    file.close();
}

bool UserDatabase::verifyCredentials(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }
    
    return it->second == password;
}

bool UserDatabase::addUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    if (users.find(username) != users.end()) {
        return false; // User already exists
    }
    
    users[username] = password;
    return true;
}

bool UserDatabase::userExists(const std::string& username) {
    std::lock_guard<std::mutex> lock(dbMutex);
    return users.find(username) != users.end();
}

} // namespace server
