#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <string>
#include <memory>
#include <unordered_map>

namespace server {

class Database;
class ClientHandler;

class UserManager {
public:
    UserManager(Database& db);

    bool verify_credentials(const std::string& username, const std::string& password);
    int get_user_id(const std::string& username);

    void add_client(int user_id, ClientHandler* client);
    void remove_client(int user_id);
    ClientHandler* get_client(int user_id);

private:
    Database& db_;
    std::unordered_map<int, ClientHandler*> active_clients_;
};

} // namespace server

#endif // USER_MANAGER_H
