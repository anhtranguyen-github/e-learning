#ifndef SERVER_MODEL_USER_H
#define SERVER_MODEL_USER_H

#include <string>

namespace server {

class User {
private:
    int id;
    std::string username;
    std::string passwordHash;
    std::string fullName;
    std::string role;
    std::string level;

public:
    User() : id(-1) {}
    User(int id, const std::string& username, const std::string& passwordHash, 
         const std::string& fullName, const std::string& role, const std::string& level)
        : id(id), username(username), passwordHash(passwordHash), 
          fullName(fullName), role(role), level(level) {}

    int getId() const { return id; }
    std::string getUsername() const { return username; }
    std::string getPasswordHash() const { return passwordHash; }
    std::string getFullName() const { return fullName; }
    std::string getRole() const { return role; }
    std::string getLevel() const { return level; }

    void setId(int id) { this->id = id; }
    void setUsername(const std::string& username) { this->username = username; }
    void setPasswordHash(const std::string& passwordHash) { this->passwordHash = passwordHash; }
    void setFullName(const std::string& fullName) { this->fullName = fullName; }
    void setRole(const std::string& role) { this->role = role; }
    void setLevel(const std::string& level) { this->level = level; }
};

} // namespace server

#endif // SERVER_MODEL_USER_H
