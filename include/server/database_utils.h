#ifndef DATABASE_UTILS_H
#define DATABASE_UTILS_H

#include <string>
#include <unordered_map>
#include <mutex>

namespace server {

class UserDatabase {
private:
    std::unordered_map<std::string, std::string> users; // username -> password
    std::mutex dbMutex;
    std::string dbFilePath;

    void loadFromFile();
    void saveToFile();

public:
    UserDatabase(const std::string& filePath = "data/users.txt");

    // Verify user credentials
    bool verifyCredentials(const std::string& username, const std::string& password);

    // Add a new user
    bool addUser(const std::string& username, const std::string& password);

    // Check if user exists
    bool userExists(const std::string& username);
};

} // namespace server

#endif // DATABASE_UTILS_H
