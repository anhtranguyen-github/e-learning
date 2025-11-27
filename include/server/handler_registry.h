#ifndef HANDLER_REGISTRY_H
#define HANDLER_REGISTRY_H

#include "server/session.h"
#include "server/connection_manager.h"
#include "server/database.h"
#include "server/controller/user_controller.h"
#include "server/controller/chat_controller.h"
#include "server/controller/lesson_controller.h"
#include "server/controller/exercise_controller.h"
#include "server/controller/submission_controller.h"
#include "server/controller/result_controller.h"
#include "server/controller/exam_controller.h"
#include "common/protocol.h"
#include <memory>
#include <map>
#include <functional>

namespace server {

class HandlerRegistry {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ConnectionManager> connectionManager;
    std::shared_ptr<Database> db;

    // Handlers (Controllers)
    std::shared_ptr<UserController> userController;
    std::shared_ptr<ChatController> chatController;
    std::shared_ptr<LessonController> lessonController;
    std::shared_ptr<ExerciseController> exerciseController;
    std::shared_ptr<SubmissionController> submissionController;
    std::shared_ptr<ResultController> resultController;
    std::shared_ptr<ExamController> examController;

public:
    HandlerRegistry(std::shared_ptr<SessionManager> sessionMgr,
                   std::shared_ptr<ConnectionManager> connMgr,
                   std::shared_ptr<Database> database);

    void handleMessage(int clientFd, const protocol::Message& msg, ClientHandler* clientHandler = nullptr);
};

} // namespace server

#endif // HANDLER_REGISTRY_H
