#ifndef FEEDBACK_CONTROLLER_H
#define FEEDBACK_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/repository/result_repository.h"
#include "server/repository/exercise_repository.h"
#include "server/repository/exam_repository.h"
#include <memory>
#include <string>

namespace server {

class FeedbackController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ResultRepository> resultRepo;
    std::shared_ptr<ExerciseRepository> exerciseRepo;
    std::shared_ptr<ExamRepository> examRepo;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    FeedbackController(std::shared_ptr<SessionManager> sessionMgr, 
                       std::shared_ptr<ResultRepository> resultRepo,
                       std::shared_ptr<ExerciseRepository> exerciseRepo,
                       std::shared_ptr<ExamRepository> examRepo);

    // Teacher gets list of all student submissions
    void handleGetSubmissions(int clientFd, const protocol::Message& msg);
    
    // Teacher submits grade for a submission
    void handleGradeSubmission(int clientFd, const protocol::Message& msg);
    
    // Teacher adds feedback (audio or text) to a result
    void handleAddFeedback(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // FEEDBACK_CONTROLLER_H
