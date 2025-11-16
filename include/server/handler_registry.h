#ifndef HANDLER_REGISTRY_H
#define HANDLER_REGISTRY_H

#include <memory>
#include "server/chat_handler.h"
#include "server/lesson_handler.h"
#include "server/exercise_handler.h"
#include "server/submission_handler.h"
#include "server/result_handler.h"
#include "server/exam_handler.h"

class UserManager;
class Database;

namespace server {

class SessionManager;

class HandlerRegistry {
public:
    HandlerRegistry(std::shared_ptr<SessionManager> sm, UserManager& user_manager, std::shared_ptr<Database> db);

    std::shared_ptr<ChatHandler> get_chat_handler();
    std::shared_ptr<LessonHandler> get_lesson_handler();
    std::shared_ptr<ExerciseHandler> get_exercise_handler();
    std::shared_ptr<SubmissionHandler> get_submission_handler();
    std::shared_ptr<ResultHandler> get_result_handler();
    std::shared_ptr<ExamHandler> get_exam_handler();

private:
    std::shared_ptr<ChatHandler> chat_handler_;
    std::shared_ptr<LessonHandler> lesson_handler_;
    std::shared_ptr<ExerciseHandler> exercise_handler_;
    std::shared_ptr<SubmissionHandler> submission_handler_;
    std::shared_ptr<ResultHandler> result_handler_;
    std::shared_ptr<ExamHandler> exam_handler_;
};

} // namespace server

#endif // HANDLER_REGISTRY_H
