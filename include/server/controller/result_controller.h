#ifndef RESULT_CONTROLLER_H
#define RESULT_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/repository/result_repository.h"
#include <memory>
#include <string>

namespace server {

class ResultController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ResultRepository> resultRepo;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    ResultController(std::shared_ptr<SessionManager> sessionMgr, 
                  std::shared_ptr<ResultRepository> repo);

    void handleResultRequest(int clientFd, const protocol::Message& msg);
    void handleDoneUndoneListRequest(int clientFd, const protocol::Message& msg);
    void handlePendingSubmissionsRequest(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // RESULT_CONTROLLER_H