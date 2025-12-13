#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/repository/game_repository.h"
#include "server/repository/result_repository.h"
#include <memory>
#include <string>

namespace server {

class GameController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<GameRepository> gameRepository;
    std::shared_ptr<ResultRepository> resultRepository;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    GameController(std::shared_ptr<SessionManager> sm, 
                   std::shared_ptr<GameRepository> gr,
                   std::shared_ptr<ResultRepository> rr);

    void handleGameListRequest(int clientFd, const protocol::Message& msg);
    void handleGameLevelListRequest(int clientFd, const protocol::Message& msg);
    void handleGameDataRequest(int clientFd, const protocol::Message& msg);
    void handleGameSubmitRequest(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // GAME_CONTROLLER_H
