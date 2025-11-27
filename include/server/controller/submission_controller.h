#ifndef SUBMISSION_CONTROLLER_H
#define SUBMISSION_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/database.h"
#include "server/repository/exercise_repository.h"
#include "server/repository/exam_repository.h"
#include <memory>
#include <string>

namespace server {

class SubmissionController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<Database> db;
    std::shared_ptr<ExerciseRepository> exerciseRepo;
    std::shared_ptr<ExamRepository> examRepo;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    SubmissionController(std::shared_ptr<SessionManager> sessionMgr, 
                      std::shared_ptr<Database> database,
                      std::shared_ptr<ExerciseRepository> exerciseRepo,
                      std::shared_ptr<ExamRepository> examRepo);

    void handleSubmission(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // SUBMISSION_CONTROLLER_H