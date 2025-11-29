#ifndef REQUEST_ROUTER_H
#define REQUEST_ROUTER_H

#include "server/session.h"
#include "server/connection_manager.h"
#include "server/database.h"
#include "server/repository/result_repository.h"
#include "server/middleware.h"
#include "common/protocol.h"
#include <memory>
#include <vector>

namespace server {

class UserController;
class ChatController;
class LessonController;
class ExerciseController;
class SubmissionController;
class ResultController;
class ExamController;
class ClientHandler;

class RequestRouter {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ConnectionManager> connectionManager;
    std::shared_ptr<Database> db;
    std::shared_ptr<ResultRepository> resultRepo;

    // Middlewares
    std::vector<std::shared_ptr<Middleware>> middlewares;

    // Controllers
    std::shared_ptr<UserController> userController;
    std::shared_ptr<ChatController> chatController;
    std::shared_ptr<LessonController> lessonController;
    std::shared_ptr<ExerciseController> exerciseController;
    std::shared_ptr<SubmissionController> submissionController;
    std::shared_ptr<ResultController> resultController;
    std::shared_ptr<ExamController> examController;

    void sendErrorResponse(int clientFd, protocol::MsgCode code, const std::string& message);

public:
    RequestRouter(std::shared_ptr<SessionManager> sessionMgr,
                  std::shared_ptr<ConnectionManager> connMgr,
                  std::shared_ptr<Database> database,
                  std::shared_ptr<ResultRepository> resultRepo);

    void registerMiddleware(std::shared_ptr<Middleware> middleware);
    void handleMessage(int clientFd, const protocol::Message& msg, ClientHandler* clientHandler = nullptr);
};

} // namespace server

#endif // REQUEST_ROUTER_H
