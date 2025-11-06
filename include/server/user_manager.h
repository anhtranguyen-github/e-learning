#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "server/database.h"
#include <string>
#include <memory>

namespace server {

class UserManager {
private:
    std::shared_ptr<Database> db;

public:
    UserManager(std::shared_ptr<Database> database);

    // User authentication and management
    bool verifyCredentials(const std::string& username, const std::string& password);
    bool addUser(const std::string& username, const std::string& password, 
                 const std::string& role = "student", const std::string& level = "beginner");
    bool userExists(const std::string& username);
    
    // Get user information
    int getUserId(const std::string& username);
    std::string getUserRole(const std::string& username);
    std::string getUserLevel(const std::string& username);
    
    // Update user information
    bool updateUserLevel(const std::string& username, const std::string& level);
    bool updatePassword(const std::string& username, const std::string& newPassword);
};

} // namespace server

#endif // USER_MANAGER_H
