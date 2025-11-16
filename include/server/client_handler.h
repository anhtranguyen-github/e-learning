#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "common/protocol.h"
#include <vector>
#include <memory>

namespace server {

class SessionManager;
class UserManager;
class HandlerRegistry;

class ClientHandler {
public:
    ClientHandler(
        std::shared_ptr<SessionManager> sm, 
        std::shared_ptr<UserManager> um,
        std::shared_ptr<HandlerRegistry> hr);

    void processMessage(int clientFd, const std::vector<uint8_t>& data);
    void handleClientDisconnect(int clientFd);
    bool send_message(const protocol::Message& msg);
    int get_user_id() const;

private:
    int clientFd_;
    std::shared_ptr<SessionManager> sessionManager_;
    std::shared_ptr<UserManager> userManager_;
    std::shared_ptr<HandlerRegistry> handlerRegistry_;

    void handleLoginRequest(const protocol::Message& msg);
    void handleLogoutRequest(const protocol::Message& msg);
    void handleHeartbeat(const protocol::Message& msg);
    void handleDisconnectRequest();
};

} // namespace server

#endif // CLIENT_HANDLER_H
