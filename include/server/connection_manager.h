#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <string>
#include <memory>
#include <unordered_map>

namespace server {

class ClientHandler;

class ConnectionManager {
public:
    ConnectionManager() = default;

    void add_client(int user_id, ClientHandler* client);
    void remove_client(int user_id);
    ClientHandler* get_client(int user_id);

private:
    std::unordered_map<int, ClientHandler*> active_clients_;
};

} // namespace server

#endif // CONNECTION_MANAGER_H
