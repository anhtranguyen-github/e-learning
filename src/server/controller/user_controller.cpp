#include "server/controller/user_controller.h"
#include "server/repository/user_repository.h"
#include "server/session.h"
#include "server/connection_manager.h"
#include "common/logger.h"
#include "common/utils.h"
#include <sys/socket.h>

namespace server {

UserController::UserController(std::shared_ptr<UserRepository> userRepo,
                               std::shared_ptr<SessionManager> sessionMgr,
                               std::shared_ptr<ConnectionManager> connMgr)
    : userRepo(userRepo), sessionMgr(sessionMgr), connMgr(connMgr) {}

void UserController::sendMessage(int clientFd, const protocol::Message& msg) {
    std::vector<uint8_t> data = msg.serialize();
    send(clientFd, data.data(), data.size(), 0);
}

void UserController::handleLoginRequest(int clientFd, const protocol::Message& msg, ClientHandler* clientHandler) {
    std::string payload = msg.toString();
    std::string username, password;
    
    if (!utils::parseLoginCredentials(payload, username, password)) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid login credentials format from fd=" + std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::LOGIN_FAILURE, "Invalid credentials format");
        sendMessage(clientFd, response);
        return;
    }

    // Verify credentials
    if (userRepo->verifyCredentials(username, password)) {
        // Get user ID
        int userId = userRepo->getUserId(username);
        
        // Create session
        std::string sessionId = sessionMgr->create_session(userId, clientFd);
        
        // Register client with ConnectionManager
        connMgr->add_client(userId, clientHandler);

        protocol::Message response(protocol::MsgCode::LOGIN_SUCCESS, "session_id=" + sessionId);
        sendMessage(clientFd, response);
        
        if (logger::serverLogger) {
            logger::serverLogger->info("User " + username + " logged in successfully (fd=" + 
                                      std::to_string(clientFd) + ")");
        }
    } else {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Failed login attempt for user " + username + 
                                      " (fd=" + std::to_string(clientFd) + ")");
        }
        protocol::Message response(protocol::MsgCode::LOGIN_FAILURE, "Invalid credentials");
        sendMessage(clientFd, response);
    }
}

void UserController::handleLogoutRequest(int clientFd, const protocol::Message& msg) {
    std::string sessionId = msg.toString();
    
    if (sessionId.empty()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("No session token in logout request from fd=" + 
                                       std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::GENERAL_FAILURE, "No session token");
        sendMessage(clientFd, response);
        return;
    }

    // Validate session
    if (!sessionMgr->is_session_valid(sessionId)) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid session token in logout request from fd=" + 
                                       std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::GENERAL_FAILURE, "Invalid session");
        sendMessage(clientFd, response);
        return;
    }

    // Unregister client
    int userId = sessionMgr->get_user_id_by_session(sessionId);
    if (userId != -1) {
        connMgr->remove_client(userId);
    }

    // Remove session
    sessionMgr->remove_session(sessionId);
    
    // Send success response
    protocol::Message response(protocol::MsgCode::LOGOUT_SUCCESS, "Logout successful");
    sendMessage(clientFd, response);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("User logged out successfully (fd=" + 
                                  std::to_string(clientFd) + ")");
    }
}

} // namespace server
