#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include "common/protocol.h"

namespace server {

class SessionManager;
class ConnectionManager;
class HandlerRegistry;

class ClientHandler {
public:
    ClientHandler(std::shared_ptr<SessionManager> sm,
                  std::shared_ptr<ConnectionManager> cm,
                  std::shared_ptr<HandlerRegistry> hr);

    void processMessage(int clientFd, const std::vector<uint8_t>& data);
    void handleClientDisconnect(int clientFd);
    bool send_message(const protocol::Message& msg);
    int get_user_id() const;

private:
    std::shared_ptr<SessionManager> sessionManager_;
    std::shared_ptr<ConnectionManager> connectionManager_;
    std::shared_ptr<HandlerRegistry> handlerRegistry_;
    int clientFd_;

    void handleHeartbeat(const protocol::Message& msg);
    void handleDisconnectRequest();
};

} // namespace server

#endif // CLIENT_HANDLER_H
