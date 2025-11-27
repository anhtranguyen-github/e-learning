#ifndef RESULT_CONTROLLER_H
#define RESULT_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/database.h"
#include <memory>
#include <string>

namespace server {

class ResultController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<Database> db;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    ResultController(std::shared_ptr<SessionManager> sessionMgr, 
                  std::shared_ptr<Database> database);

    void handleResultRequest(int clientFd, const protocol::Message& msg);
    void handleDoneUndoneListRequest(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // RESULT_CONTROLLER_H