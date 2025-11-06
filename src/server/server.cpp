#include "server/server.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <csignal>
#include <iostream>


#include "server/lesson_loader.h"
#include "server/lesson_handler.h"


namespace server {

Server::Server(int port, const std::string& dbConn)
    : serverSocket(-1), port(port), running(false), dbConnInfo(dbConn) {
    
    // Initialize database connection
    database = std::make_shared<Database>(dbConnInfo);
    if (!database->connect()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to connect to database");
        }
        throw std::runtime_error("Database connection failed");
    }
    
    // Initialize managers
    userManager = std::make_shared<UserManager>(database);
    sessionManager = std::make_shared<SessionManager>(database, 30);

    auto lessonLoader = std::make_shared<LessonLoader>(database);

    lessonHandler = std::make_shared<LessonHandler>(sessionManager, lessonLoader);
    
    clientHandler = std::make_shared<ClientHandler>(sessionManager, userManager, lessonHandler);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Server components initialized successfully");
    }
}

Server::~Server() {
    stop();
}

bool Server::initSocket() {
    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to create server socket");
        }
        return false;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to set socket options");
        }
        close(serverSocket);
        return false;
    }

    // Set non-blocking
    int flags = fcntl(serverSocket, F_GETFL, 0);
    fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

    // Bind socket
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to bind socket to port " + std::to_string(port));
        }
        close(serverSocket);
        return false;
    }

    // Listen
    if (listen(serverSocket, 10) < 0) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to listen on socket");
        }
        close(serverSocket);
        return false;
    }

    if (logger::serverLogger) {
        logger::serverLogger->info("Server socket initialized on port " + std::to_string(port));
    }

    return true;
}

int Server::acceptClient() {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    
    int clientFd = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
    
    if (clientFd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            if (logger::serverLogger) {
                logger::serverLogger->error("Failed to accept client connection");
            }
        }
        return -1;
    }

    // Set non-blocking
    int flags = fcntl(clientFd, F_GETFL, 0);
    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

    clientSockets.insert(clientFd);

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("New client connected: " + std::string(clientIp) + 
                                  " (fd=" + std::to_string(clientFd) + ")");
    }

    return clientFd;
}

void Server::handleClientData(int clientFd) {
    std::vector<uint8_t> buffer(4096);
    
    ssize_t received = recv(clientFd, buffer.data(), buffer.size(), 0);
    
    if (received <= 0) {
        if (received == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
            // Client disconnected
            removeClient(clientFd);
        }
        return;
    }

    buffer.resize(received);
    
    // Process the message
    clientHandler->processMessage(clientFd, buffer);
}

void Server::removeClient(int clientFd) {
    clientHandler->handleClientDisconnect(clientFd);
    clientSockets.erase(clientFd);
    close(clientFd);
}

bool Server::start() {
    if (!initSocket()) {
        return false;
    }

    running = true;
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Server started on port " + std::to_string(port));
    }

    return true;
}

void Server::run() {
    if (!running) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Server not started");
        }
        return;
    }

    if (logger::serverLogger) {
        logger::serverLogger->info("Server entering main event loop");
    }

    auto lastSessionCheck = std::chrono::steady_clock::now();
    const int sessionCheckInterval = 5; // seconds

    while (running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        
        // Add server socket
        FD_SET(serverSocket, &readfds);
        int maxFd = serverSocket;
        
        // Add all client sockets
        for (int clientFd : clientSockets) {
            FD_SET(clientFd, &readfds);
            if (clientFd > maxFd) {
                maxFd = clientFd;
            }
        }

        // Set timeout for select
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(maxFd + 1, &readfds, nullptr, nullptr, &timeout);

        if (activity < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (logger::serverLogger) {
                logger::serverLogger->error("Select error");
            }
            break;
        }

        // Check for new connections
        if (FD_ISSET(serverSocket, &readfds)) {
            int newClient = acceptClient();
            if (newClient >= 0) {
                // Client accepted
            }
        }

        // Check for client data
        std::vector<int> clientsToCheck(clientSockets.begin(), clientSockets.end());
        for (int clientFd : clientsToCheck) {
            if (FD_ISSET(clientFd, &readfds)) {
                handleClientData(clientFd);
            }
        }

        // Periodically check for expired sessions
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastSessionCheck).count();
        
        if (elapsed >= sessionCheckInterval) {
            sessionManager->checkExpiredSessions();
            lastSessionCheck = now;
        }
    }

    if (logger::serverLogger) {
        logger::serverLogger->info("Server event loop terminated");
    }
}

void Server::stop() {
    running = false;

    // Close all client connections
    for (int clientFd : clientSockets) {
        close(clientFd);
    }
    clientSockets.clear();

    // Close server socket
    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }

    if (logger::serverLogger) {
        logger::serverLogger->info("Server stopped");
    }
}

} // namespace server

// Main entry point
int main(int argc, char* argv[]) {
    // Initialize logger
    logger::initServerLogger();

    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }

    server::Server srv(port);

    // Handle Ctrl+C gracefully
    signal(SIGINT, [](int) {
        std::cout << "\nShutting down server..." << std::endl;
        exit(0);
    });

    if (!srv.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    srv.run();

    return 0;
}
