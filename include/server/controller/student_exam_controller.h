#ifndef STUDENT_EXAM_CONTROLLER_H
#define STUDENT_EXAM_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/repository/exam_repository.h"
#include "server/repository/result_repository.h"
#include <memory>
#include <string>

namespace server {

class StudentExamController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ExamRepository> examRepository;
    std::shared_ptr<ResultRepository> resultRepository;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    StudentExamController(std::shared_ptr<SessionManager> sessionMgr, 
                          std::shared_ptr<ExamRepository> examRepo,
                          std::shared_ptr<ResultRepository> resultRepo);

    // Student gets list of exams
    void handleGetExams(int clientFd, const protocol::Message& msg);
    
    // Student requests exam content (with "already taken" check)
    void handleExamRequest(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // STUDENT_EXAM_CONTROLLER_H
