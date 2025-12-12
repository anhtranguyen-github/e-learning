#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include "server/database.h"
#include "server/model/user.h"
#include <memory>
#include <string>

namespace server {

class UserRepository {
private:
    std::shared_ptr<Database> db;

public:
    UserRepository(std::shared_ptr<Database> database);

    // Verify username and password
    bool verifyCredentials(const std::string& username, const std::string& password);

    // Get user ID by username
    int getUserId(const std::string& username);

    // Find user by ID (optional, but good for completeness)
    User findById(int id);

    // Create a new user (for registration)
    bool createUser(const std::string& username, const std::string& password, const std::string& role = "student");

    // Check if username exists
    bool usernameExists(const std::string& username);
};

} // namespace server

#endif // USER_REPOSITORY_H
