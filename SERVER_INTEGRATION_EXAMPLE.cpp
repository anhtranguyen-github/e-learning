/**
 * SERVER_INTEGRATION_EXAMPLE.cpp
 * 
 * This example shows how to integrate the lesson management components
 * into your existing TCP server with multiplexing.
 */

#include "server/server.h"
#include "server/database.h"
#include "server/session.h"
#include "server/user_manager.h"
#include "server/client_handler.h"
#include "server/lesson_loader.h"
#include "server/lesson_handler.h"
#include "common/logger.h"

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include <algorithm>

using namespace server;

int main() {
    // Initialize server logger
    logger::initServerLogger();
    
    // ========================================================================
    // STEP 1: Initialize Database Connection
    // ========================================================================
    std::string dbConnectionString = "host=localhost port=5432 dbname=learning_app "
                                     "user=youruser password=yourpassword";
    
    auto database = std::make_shared<Database>(dbConnectionString);
    
    if (!database->connect()) {
        logger::serverLogger->error("Failed to connect to database");
        return 1;
    }
    
    logger::serverLogger->info("Connected to PostgreSQL database");
    
    // ========================================================================
    // STEP 2: Initialize Core Components
    // ========================================================================
    
    // Session manager (handles authentication tokens, heartbeat timeouts)
    auto sessionManager = std::make_shared<SessionManager>(database, 30);
    
    // User manager (handles login/logout authentication)
    auto userManager = std::make_shared<UserManager>(database);
    
    // ========================================================================
    // STEP 3: Initialize Lesson Components (NEW)
    // ========================================================================
    
    // Lesson loader - loads lessons from database
    auto lessonLoader = std::make_shared<LessonLoader>(database);
    
    // Lesson handler - processes lesson requests from clients
    auto lessonHandler = std::make_shared<LessonHandler>(sessionManager, lessonLoader);
    
    logger::serverLogger->info("Lesson management components initialized");
    
    // ========================================================================
    // STEP 4: Initialize Client Handler with Lesson Support
    // ========================================================================
    
    // Client handler - routes all client messages
    auto clientHandler = std::make_shared<ClientHandler>(sessionManager, userManager);
    
    // IMPORTANT: Inject lesson handler into client handler
    clientHandler->setLessonHandler(lessonHandler);
    
    logger::serverLogger->info("Client handler initialized with lesson support");
    
    // ========================================================================
    // STEP 5: Setup TCP Server Socket
    // ========================================================================
    
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        logger::serverLogger->error("Failed to create server socket");
        return 1;
    }
    
    // Set socket options (reuse address)
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind to port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        logger::serverLogger->error("Failed to bind server socket");
        close(serverSocket);
        return 1;
    }
    
    // Listen for connections
    if (listen(serverSocket, 10) < 0) {
        logger::serverLogger->error("Failed to listen on server socket");
        close(serverSocket);
        return 1;
    }
    
    logger::serverLogger->info("Server listening on port 8080");
    
    // ========================================================================
    // STEP 6: Main Server Loop with Multiplexing
    // ========================================================================
    
    std::vector<int> clientSockets;
    bool running = true;
    
    while (running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        
        // Add server socket
        FD_SET(serverSocket, &readfds);
        int maxfd = serverSocket;
        
        // Add all client sockets
        for (int clientFd : clientSockets) {
            FD_SET(clientFd, &readfds);
            if (clientFd > maxfd) {
                maxfd = clientFd;
            }
        }
        
        // Wait for activity on any socket
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(maxfd + 1, &readfds, nullptr, nullptr, &timeout);
        
        if (activity < 0) {
            logger::serverLogger->error("Select error");
            break;
        }
        
        // Check for new connection
        if (FD_ISSET(serverSocket, &readfds)) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int clientFd = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            
            if (clientFd >= 0) {
                clientSockets.push_back(clientFd);
                logger::serverLogger->info("New client connected: fd=" + std::to_string(clientFd));
            }
        }
        
        // Check for client data
        for (auto it = clientSockets.begin(); it != clientSockets.end(); ) {
            int clientFd = *it;
            
            if (FD_ISSET(clientFd, &readfds)) {
                std::vector<uint8_t> buffer(4096);
                ssize_t bytesRead = recv(clientFd, buffer.data(), buffer.size(), 0);
                
                if (bytesRead <= 0) {
                    // Client disconnected
                    logger::serverLogger->info("Client disconnected: fd=" + std::to_string(clientFd));
                    clientHandler->handleClientDisconnect(clientFd);
                    close(clientFd);
                    it = clientSockets.erase(it);
                } else {
                    // Process message
                    buffer.resize(bytesRead);
                    
                    // This will automatically route to lesson handler if needed
                    clientHandler->processMessage(clientFd, buffer);
                    
                    ++it;
                }
            } else {
                ++it;
            }
        }
        
        // Check for expired sessions (heartbeat timeout)
        sessionManager->checkExpiredSessions();
    }
    
    // ========================================================================
    // STEP 7: Cleanup
    // ========================================================================
    
    logger::serverLogger->info("Shutting down server");
    
    // Close all client connections
    for (int clientFd : clientSockets) {
        close(clientFd);
    }
    
    // Close server socket
    close(serverSocket);
    
    // Disconnect database
    database->disconnect();
    
    logger::serverLogger->info("Server shutdown complete");
    
    return 0;
}

/**
 * COMPILATION:
 * 
 * g++ -o server SERVER_INTEGRATION_EXAMPLE.cpp \
 *     src/server/server.cpp \
 *     src/server/database.cpp \
 *     src/server/session.cpp \
 *     src/server/user_manager.cpp \
 *     src/server/client_handler.cpp \
 *     src/server/lesson_loader.cpp \
 *     src/server/lesson_handler.cpp \
 *     src/common/protocol.cpp \
 *     src/common/logger.cpp \
 *     src/common/utils.cpp \
 *     -Iinclude \
 *     -lpq -ljsoncpp -lpthread -std=c++17
 * 
 * RUN:
 * ./server
 * 
 * The server will now handle:
 * - LOGIN_REQUEST (existing)
 * - LOGOUT_REQUEST (existing)
 * - HEARTBEAT (existing)
 * - DISCONNECT_REQUEST (existing)
 * - LESSON_LIST_REQUEST (NEW)
 * - STUDY_LESSON_REQUEST (NEW)
 */
