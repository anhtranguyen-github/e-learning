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
    ssize_t sent = send(clientFd, data.data(), data.size(), 0);
    if (logger::serverLogger) {
        if (sent < 0) {
            logger::serverLogger->error("Failed to send message to fd=" + std::to_string(clientFd) + ", error=" + std::string(strerror(errno)));
        } else {
            logger::serverLogger->debug("Sent " + std::to_string(sent) + " bytes to fd=" + std::to_string(clientFd));
        }
    }
}

void UserController::handleUserLoginRequest(int clientFd, const protocol::Message& msg, ClientHandler* clientHandler) {
    if (logger::serverLogger) logger::serverLogger->debug("Handling login request for fd=" + std::to_string(clientFd));
    
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

    if (logger::serverLogger) logger::serverLogger->debug("Verifying credentials for user: " + username);

    // Verify credentials
    if (userRepo->verifyCredentials(username, password)) {
        if (logger::serverLogger) logger::serverLogger->debug("Credentials verified, creating session");
        
        // Get user ID
        int userId = userRepo->getUserId(username);
        
        // Get user details to retrieve role
        User user = userRepo->findById(userId);
        std::string role = user.getRole();
        
        // Create session with role
        std::string sessionId = sessionMgr->create_session(userId, clientFd, role);
        
        // Register client with ConnectionManager
        connMgr->add_client(userId, clientHandler);

        protocol::Message response(protocol::MsgCode::LOGIN_SUCCESS, "session_id=" + sessionId + ";role=" + role);
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

void UserController::handleUserLogoutRequest(int clientFd, const protocol::Message& msg) {
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

void UserController::handleUserRegisterRequest(int clientFd, const protocol::Message& msg) {
    if (logger::serverLogger) logger::serverLogger->debug("Handling register request for fd=" + std::to_string(clientFd));
    
    std::string payload = msg.toString();
    std::string username, password;
    
    // Parse registration data (same format as login: username;password)
    if (!utils::parseLoginCredentials(payload, username, password)) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid registration format from fd=" + std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::REGISTER_FAILURE, "Invalid registration format");
        sendMessage(clientFd, response);
        return;
    }

    // Validate username and password
    if (username.length() < 3 || password.length() < 4) {
        protocol::Message response(protocol::MsgCode::REGISTER_FAILURE, "Username must be at least 3 characters and password at least 4 characters");
        sendMessage(clientFd, response);
        return;
    }

    // Create user
    if (userRepo->createUser(username, password, "student")) {
        protocol::Message response(protocol::MsgCode::REGISTER_SUCCESS, "Registration successful");
        sendMessage(clientFd, response);
        
        if (logger::serverLogger) {
            logger::serverLogger->info("User " + username + " registered successfully (fd=" + 
                                      std::to_string(clientFd) + ")");
        }
    } else {
        protocol::Message response(protocol::MsgCode::REGISTER_FAILURE, "Username already exists or registration failed");
        sendMessage(clientFd, response);
        
        if (logger::serverLogger) {
            logger::serverLogger->warn("Failed registration attempt for user " + username + 
                                      " (fd=" + std::to_string(clientFd) + ")");
        }
    }
}

} // namespace server
