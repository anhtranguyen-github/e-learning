#include "server/handler_registry.h"
#include "server/session.h"
#include "server/chat_handler.h"
#include "server/lesson_handler.h"
#include "server/exercise_handler.h"
#include "server/submission_handler.h"
#include "server/result_handler.h"
#include "server/exam_handler.h"
#include "server/lesson_loader.h"
#include "server/exercise_loader.h"
#include "server/exam_loader.h"

namespace server {

HandlerRegistry::HandlerRegistry(std::shared_ptr<SessionManager> sm, UserManager& user_manager, std::shared_ptr<Database> db)
    : chat_handler_(std::make_shared<ChatHandler>(user_manager, *db))
{
    auto lesson_loader = std::make_shared<LessonLoader>(db);
    lesson_handler_ = std::make_shared<LessonHandler>(sm, lesson_loader);

    auto exercise_loader = std::make_shared<ExerciseLoader>(db);
    exercise_handler_ = std::make_shared<ExerciseHandler>(sm, exercise_loader);

    submission_handler_ = std::make_shared<SubmissionHandler>(sm, db);
    result_handler_ = std::make_shared<ResultHandler>(sm, db);

    auto exam_loader = std::make_shared<ExamLoader>(db);
    exam_handler_ = std::make_shared<ExamHandler>(sm, exam_loader);
}

std::shared_ptr<ChatHandler> HandlerRegistry::get_chat_handler() { return chat_handler_; }
std::shared_ptr<LessonHandler> HandlerRegistry::get_lesson_handler() { return lesson_handler_; }
std::shared_ptr<ExerciseHandler> HandlerRegistry::get_exercise_handler() { return exercise_handler_; }
std::shared_ptr<SubmissionHandler> HandlerRegistry::get_submission_handler() { return submission_handler_; }
std::shared_ptr<ResultHandler> HandlerRegistry::get_result_handler() { return result_handler_; }
std::shared_ptr<ExamHandler> HandlerRegistry::get_exam_handler() { return exam_handler_; }

} // namespace server
