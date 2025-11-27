#include "server/client_handler.h"
#include "server/session.h"
#include "server/connection_manager.h"
#include "server/request_router.h"
#include "common/logger.h"
#include "common/utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

namespace server {

ClientHandler::ClientHandler(
    std::shared_ptr<SessionManager> sm,
    std::shared_ptr<ConnectionManager> cm,
    std::shared_ptr<RequestRouter> rr)
    : sessionManager_(sm), connectionManager_(cm), requestRouter_(rr) {}

void ClientHandler::processMessage(int clientFd, const std::vector<uint8_t>& data) {
    clientFd_ = clientFd;
    protocol::Message response; // Declare response here
    
    if (data.empty()) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Received empty data from fd=" + std::to_string(clientFd));
        }
        return;
    }

    try {
        protocol::Message msg = protocol::Message::deserialize(data);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("Received message code: " + std::to_string(static_cast<uint16_t>(msg.code)) +
                                      " from fd=" + std::to_string(clientFd));
        }

            switch (msg.code) {
            case protocol::MsgCode::HEARTBEAT:
                handleHeartbeat(msg);
                break;
            case protocol::MsgCode::DISCONNECT_REQUEST:
                handleDisconnectRequest();
                break;

            default:
                // Delegate all other messages (including LOGIN/LOGOUT) to the RequestRouter
                requestRouter_->handleMessage(clientFd, msg, this);
                break;
        }
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Error processing message from fd=" + std::to_string(clientFd) + ": " + e.what());
        }
        response = protocol::Message(protocol::MsgCode::GENERAL_FAILURE, "Server error processing message");
        send_message(response);
    }
}


void ClientHandler::handleHeartbeat(const protocol::Message& msg) {
    std::string sessionId = msg.toString();
    
    if (sessionId.empty()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("No session token in heartbeat from fd=" + 
                                       std::to_string(clientFd_));
        }
        return;
    }

    // Validate and update session
    if (sessionManager_->is_session_valid(sessionId)) {
        sessionManager_->update_session(sessionId);
        
        if (logger::heartbeatLogger) {
            logger::heartbeatLogger->info("Heartbeat from session " + sessionId);
        }
    } else {
        if (logger::heartbeatLogger) {
            logger::heartbeatLogger->warn("Invalid session in heartbeat from fd=" + 
                                      std::to_string(clientFd_));
        }
    }
}

void ClientHandler::handleDisconnectRequest() {
    // Send acknowledgment
    protocol::Message response(protocol::MsgCode::DISCONNECT_ACK, "Disconnect acknowledged");
    send_message(response);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Disconnect request from fd=" + std::to_string(clientFd_));
    }
}

void ClientHandler::handleClientDisconnect(int clientFd) {
    // Unregister client
    int userId = sessionManager_->get_user_id_by_fd(clientFd);
    if (userId != -1) {
        connectionManager_->remove_client(userId);
    }

    // Remove session if exists
    sessionManager_->remove_session_by_fd(clientFd);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Client disconnected (fd=" + std::to_string(clientFd) + ")");
    }
}

bool ClientHandler::send_message(const protocol::Message& msg) {
    std::vector<uint8_t> data = msg.serialize();
    
    ssize_t sent = send(clientFd_, data.data(), data.size(), 0);
    
    if (sent < 0) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to send message to fd=" + std::to_string(clientFd_));
        }
        return false;
    }
    
    return true;
}

int ClientHandler::get_user_id() const {
    return sessionManager_->get_user_id_by_fd(clientFd_);
}

} // namespace server

