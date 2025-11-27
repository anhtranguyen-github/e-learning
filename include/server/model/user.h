#ifndef SERVER_MODEL_USER_H
#define SERVER_MODEL_USER_H

#include <string>

namespace server {

class User {
private:
    int id;
    std::string username;
    std::string passwordHash;
    std::string salt;

public:
    User() : id(-1) {}
    User(int id, const std::string& username, const std::string& passwordHash, const std::string& salt)
        : id(id), username(username), passwordHash(passwordHash), salt(salt) {}

    int getId() const { return id; }
    std::string getUsername() const { return username; }
    std::string getPasswordHash() const { return passwordHash; }
    std::string getSalt() const { return salt; }

    void setId(int id) { this->id = id; }
    void setUsername(const std::string& username) { this->username = username; }
    void setPasswordHash(const std::string& passwordHash) { this->passwordHash = passwordHash; }
    void setSalt(const std::string& salt) { this->salt = salt; }
};

} // namespace server

#endif // SERVER_MODEL_USER_H
