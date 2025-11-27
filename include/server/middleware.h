#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include "common/protocol.h"
#include "common/logger.h"
#include "server/session.h"
#include <string>
#include <memory>
#include <iostream>

namespace server {

class Middleware {
public:
    virtual ~Middleware() = default;
    // Returns true if processing should continue, false if request should be rejected
    virtual bool handle(int clientFd, const protocol::Message& msg, std::string& errorMsg) = 0;
};

class LoggingMiddleware : public Middleware {
public:
    bool handle(int clientFd, const protocol::Message& msg, std::string& /*errorMsg*/) override {
        if (logger::serverLogger) {
            logger::serverLogger->info("Request: Code=" + std::to_string(static_cast<int>(msg.code)) + 
                                      ", Size=" + std::to_string(msg.serialize().size()) + 
                                      ", Fd=" + std::to_string(clientFd));
        }
        return true;
    }
};

class AuthMiddleware : public Middleware {
private:
    std::shared_ptr<SessionManager> sessionManager;

public:
    AuthMiddleware(std::shared_ptr<SessionManager> sm) : sessionManager(sm) {}

    bool handle(int clientFd, const protocol::Message& msg, std::string& errorMsg) override {
        // Skip auth check for Login/Register/Disconnect/Heartbeat
        if (msg.code == protocol::MsgCode::LOGIN_REQUEST || 
            msg.code == protocol::MsgCode::DISCONNECT_REQUEST ||
            msg.code == protocol::MsgCode::HEARTBEAT) {
            return true;
        }

        // For other requests, extract session token
        // This is tricky because the session token location varies by message type.
        // However, most requests (except Login) should probably include it or we rely on the fact that
        // the client MUST be logged in.
        
        // A better approach for AuthMiddleware in this specific protocol:
        // Check if the clientFd maps to a valid session in SessionManager.
        // SessionManager::get_user_id_by_fd(clientFd) returns -1 if not logged in.
        
        int userId = sessionManager->get_user_id_by_fd(clientFd);
        if (userId == -1) {
            errorMsg = "Unauthorized: Login required";
            return false;
        }
        
        return true;
    }
};

} // namespace server

#endif // MIDDLEWARE_H
