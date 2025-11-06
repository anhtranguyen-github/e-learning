#ifndef RESULT_HANDLER_H
#define RESULT_HANDLER_H

#include "server/session.h"
#include "server/database.h"
#include "common/protocol.h"
#include <memory>

namespace server {

class ResultHandler {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<Database> db;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    ResultHandler(std::shared_ptr<SessionManager> sm, std::shared_ptr<Database> db);

    void handleResultRequest(int clientFd, const protocol::Message& msg);
    void handleDoneUndoneListRequest(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // RESULT_HANDLER_H