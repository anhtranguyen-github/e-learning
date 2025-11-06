#ifndef SUBMISSION_HANDLER_H
#define SUBMISSION_HANDLER_H

#include "server/session.h"
#include "server/database.h"
#include "common/protocol.h"
#include <memory>

namespace server {

class SubmissionHandler {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<Database> db;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    SubmissionHandler(std::shared_ptr<SessionManager> sm, std::shared_ptr<Database> db);

    void handleSubmission(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // SUBMISSION_HANDLER_H