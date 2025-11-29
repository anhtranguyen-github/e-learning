#ifndef SERVER_H
#define SERVER_H

#include "server/session.h"
#include "server/database.h"
#include "server/connection_manager.h"
#include "server/request_router.h"
#include "server/client_handler.h"
#include <vector>
#include <set>
#include <map>
#include <memory>

class Server {
private:
    int serverSocket;
    int port;
    bool running;
    std::string dbConnInfo;
    
    std::shared_ptr<server::Database> database;
    std::shared_ptr<server::ConnectionManager> connectionManager;
    std::shared_ptr<server::SessionManager> sessionManager;
    
    // Repositories
    std::shared_ptr<server::ResultRepository> resultRepository;
    
    std::shared_ptr<server::ClientHandler> clientHandler;
    std::shared_ptr<server::RequestRouter> requestRouter;

    
    std::set<int> clientSockets;
    std::map<int, std::vector<uint8_t>> clientBuffers;
    
    // Initialize server socket
    bool initSocket();
    
    // Accept new client connection
    int acceptClient();
    
    // Handle client data
    void handleClientData(int clientFd);
    
    // Remove client
    void removeClient(int clientFd);

public:
    Server(int port = 8080, const std::string& dbConn = "host=localhost port=5432 dbname=english_learning user=postgres password=yourpass");
    ~Server();

    // Start server
    bool start();
    
    // Main event loop with select()
    void run();
    
    // Stop server
    void stop();
};

#endif // SERVER_H
