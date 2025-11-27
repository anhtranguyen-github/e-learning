#ifndef EXAM_CONTROLLER_H
#define EXAM_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/repository/exam_repository.h"
#include <memory>
#include <string>

namespace server {

class ExamController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ExamRepository> examRepository;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    ExamController(std::shared_ptr<SessionManager> sessionMgr, 
                  std::shared_ptr<ExamRepository> examRepo);

    void handleGetExams(int clientFd, const protocol::Message& msg);
    void handleExamRequest(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // EXAM_CONTROLLER_H
