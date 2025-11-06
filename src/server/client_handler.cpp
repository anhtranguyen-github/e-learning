#include "server/client_handler.h"
#include "common/logger.h"
#include "common/utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

namespace server {

ClientHandler::ClientHandler(SessionManager& sm, UserDatabase& db)
    : sessionManager(sm), userDatabase(db) {
}

void ClientHandler::processMessage(int clientFd, const std::vector<uint8_t>& data) {
    try {
        protocol::Message msg = protocol::Message::deserialize(data);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("Received message code " + 
                                       std::to_string(static_cast<int>(msg.code)) + 
                                       " from fd=" + std::to_string(clientFd));
        }

        switch (msg.code) {
            case protocol::MsgCode::LOGIN_REQUEST:
                handleLoginRequest(clientFd, msg);
                break;
            
            case protocol::MsgCode::LOGOUT_REQUEST:
                handleLogoutRequest(clientFd, msg);
                break;
            
            case protocol::MsgCode::HEARTBEAT:
                handleHeartbeat(clientFd, msg);
                break;
            
            case protocol::MsgCode::DISCONNECT_REQUEST:
                handleDisconnectRequest(clientFd);
                break;
            
            default:
                if (logger::serverLogger) {
                    logger::serverLogger->warn("Unknown message code from fd=" + std::to_string(clientFd));
                }
                protocol::Message errorMsg(protocol::MsgCode::UNKNOWN_COMMAND_FAILURE, "Unknown command");
                sendMessage(clientFd, errorMsg);
                break;
        }
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Error processing message: " + std::string(e.what()));
        }
    }
}

void ClientHandler::handleLoginRequest(int clientFd, const protocol::Message& msg) {
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
    if (userDatabase.verifyCredentials(username, password)) {
        // Create session
        std::string sessionId = sessionManager.createSession(username, clientFd);
        
        // Send success response with cookie
        std::string cookieHeader = utils::createCookieHeader(sessionId);
        protocol::Message response(protocol::MsgCode::LOGIN_SUCCESS, cookieHeader);
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

void ClientHandler::handleLogoutRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    std::string sessionId = utils::extractCookie(payload, "session_id");
    
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
    if (!sessionManager.validateSession(sessionId)) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid session token in logout request from fd=" + 
                                       std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::GENERAL_FAILURE, "Invalid session");
        sendMessage(clientFd, response);
        return;
    }

    // Remove session
    sessionManager.removeSession(sessionId);
    
    // Send success response
    protocol::Message response(protocol::MsgCode::LOGOUT_SUCCESS, "Logout successful");
    sendMessage(clientFd, response);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("User logged out successfully (fd=" + 
                                  std::to_string(clientFd) + ")");
    }
}

void ClientHandler::handleHeartbeat(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    std::string sessionId = utils::extractCookie(payload, "session_id");
    
    if (sessionId.empty()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("No session token in heartbeat from fd=" + 
                                       std::to_string(clientFd));
        }
        return;
    }

    // Validate and update session
    if (sessionManager.validateSession(sessionId)) {
        sessionManager.updateLastActive(sessionId);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("Heartbeat received from fd=" + std::to_string(clientFd));
        }
    } else {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Invalid session in heartbeat from fd=" + 
                                      std::to_string(clientFd));
        }
    }
}

void ClientHandler::handleDisconnectRequest(int clientFd) {
    // Send acknowledgment
    protocol::Message response(protocol::MsgCode::DISCONNECT_ACK, "Disconnect acknowledged");
    sendMessage(clientFd, response);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Disconnect request from fd=" + std::to_string(clientFd));
    }
}

void ClientHandler::handleClientDisconnect(int clientFd) {
    // Remove session if exists
    sessionManager.removeSessionByFd(clientFd);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Client disconnected (fd=" + std::to_string(clientFd) + ")");
    }
}

bool ClientHandler::sendMessage(int clientFd, const protocol::Message& msg) {
    std::vector<uint8_t> data = msg.serialize();
    
    ssize_t sent = send(clientFd, data.data(), data.size(), 0);
    
    if (sent < 0) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to send message to fd=" + std::to_string(clientFd));
        }
        return false;
    }
    
    return true;
}

protocol::Message ClientHandler::receiveMessage(int clientFd) {
    std::vector<uint8_t> buffer(4096);
    
    ssize_t received = recv(clientFd, buffer.data(), buffer.size(), 0);
    
    if (received <= 0) {
        throw std::runtime_error("Failed to receive message");
    }
    
    buffer.resize(received);
    return protocol::Message::deserialize(buffer);
}

} // namespace server
