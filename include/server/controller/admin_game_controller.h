#ifndef ADMIN_GAME_CONTROLLER_H
#define ADMIN_GAME_CONTROLLER_H

#include <memory>
#include "server/session.h"
#include "server/repository/game_repository.h"
#include "common/protocol.h"

namespace server {

class AdminGameController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<GameRepository> gameRepository;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    AdminGameController(std::shared_ptr<SessionManager> sm, 
                        std::shared_ptr<GameRepository> gr);

    void handleGameCreateRequest(int clientFd, const protocol::Message& msg);
    void handleGameUpdateRequest(int clientFd, const protocol::Message& msg);
    void handleGameDeleteRequest(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // ADMIN_GAME_CONTROLLER_H
