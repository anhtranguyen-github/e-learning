#ifndef TEACHER_EXAM_CONTROLLER_H
#define TEACHER_EXAM_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/repository/exam_repository.h"
#include <memory>
#include <string>

namespace server {

class TeacherExamController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ExamRepository> examRepository;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    TeacherExamController(std::shared_ptr<SessionManager> sessionMgr, 
                          std::shared_ptr<ExamRepository> examRepo);

    // Teacher views exam content (no "already taken" check - for grading)
    void handleExamReview(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // TEACHER_EXAM_CONTROLLER_H
