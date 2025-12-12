#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include "common/protocol.h"
#include <memory>
#include <string>

namespace server {

class UserRepository;
class SessionManager;
class ConnectionManager;
class ClientHandler;

class UserController {
public:
    UserController(std::shared_ptr<UserRepository> userRepo,
                   std::shared_ptr<SessionManager> sessionMgr,
                   std::shared_ptr<ConnectionManager> connMgr);

    void handleUserLoginRequest(int clientFd, const protocol::Message& msg, ClientHandler* clientHandler);
    void handleUserLogoutRequest(int clientFd, const protocol::Message& msg);
    void handleUserRegisterRequest(int clientFd, const protocol::Message& msg);

private:
    std::shared_ptr<UserRepository> userRepo;
    std::shared_ptr<SessionManager> sessionMgr;
    std::shared_ptr<ConnectionManager> connMgr;

    void sendMessage(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // USER_CONTROLLER_H
