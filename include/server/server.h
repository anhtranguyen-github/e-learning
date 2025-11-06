#ifndef SERVER_H
#define SERVER_H

#include "server/session.h"
#include "server/database_utils.h"
#include "server/client_handler.h"
#include <vector>
#include <set>

namespace server {

class Server {
private:
    int serverSocket;
    int port;
    bool running;
    
    SessionManager sessionManager;
    UserDatabase userDatabase;
    ClientHandler clientHandler;
    
    std::set<int> clientSockets;
    
    // Initialize server socket
    bool initSocket();
    
    // Accept new client connection
    int acceptClient();
    
    // Handle client data
    void handleClientData(int clientFd);
    
    // Remove client
    void removeClient(int clientFd);

public:
    Server(int port = 8080);
    ~Server();

    // Start server
    bool start();
    
    // Main event loop with select()
    void run();
    
    // Stop server
    void stop();
};

} // namespace server

#endif // SERVER_H
