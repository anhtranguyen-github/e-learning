#include "server/request_router.h"
#include "server/controller/user_controller.h"
#include "server/controller/chat_controller.h"
#include "server/controller/lesson_controller.h"
#include "server/controller/exercise_controller.h"
#include "server/controller/submission_controller.h"
#include "server/controller/result_controller.h"
#include "server/controller/exam_controller.h"
#include "server/repository/user_repository.h"
#include "server/repository/lesson_repository.h"
#include "server/repository/exercise_repository.h"
#include "server/repository/exam_repository.h"
#include "common/logger.h"
#include "common/payloads.h"
#include <sys/socket.h>

namespace server {

RequestRouter::RequestRouter(std::shared_ptr<SessionManager> sessionMgr,
                             std::shared_ptr<ConnectionManager> connMgr,
                             std::shared_ptr<Database> database,
                             std::shared_ptr<ResultRepository> resultRepo)
    : sessionManager(sessionMgr), connectionManager(connMgr), db(database), resultRepo(resultRepo) {
    
    // Initialize Repositories
    auto userRepo = std::make_shared<UserRepository>(db);
    auto lessonRepo = std::make_shared<LessonRepository>(db);
    auto exerciseRepo = std::make_shared<ExerciseRepository>(db);
    auto examRepo = std::make_shared<ExamRepository>(db);

    // Initialize Controllers
    userController = std::make_shared<UserController>(userRepo, sessionManager, connectionManager);
    chatController = std::make_shared<ChatController>(sessionManager, connectionManager, db);
    lessonController = std::make_shared<LessonController>(sessionManager, lessonRepo);
    exerciseController = std::make_shared<ExerciseController>(sessionManager, exerciseRepo);
    submissionController = std::make_shared<SubmissionController>(sessionManager, resultRepo, exerciseRepo, examRepo);
    resultController = std::make_shared<ResultController>(sessionManager, resultRepo);
    examController = std::make_shared<ExamController>(sessionManager, examRepo, resultRepo);

    // Register Default Middlewares
    registerMiddleware(std::make_shared<LoggingMiddleware>());
    registerMiddleware(std::make_shared<AuthMiddleware>(sessionManager));
}

void RequestRouter::registerMiddleware(std::shared_ptr<Middleware> middleware) {
    middlewares.push_back(middleware);
}

void RequestRouter::sendErrorResponse(int clientFd, protocol::MsgCode code, const std::string& message) {
    Payloads::GenericResponse resp;
    resp.success = false;
    resp.message = message;
    
    // Use the provided error code
    protocol::Message error_msg(code, resp.serialize());
    
    std::vector<uint8_t> data = error_msg.serialize();
    send(clientFd, data.data(), data.size(), 0);
}

void RequestRouter::handleMessage(int clientFd, const protocol::Message& msg, ClientHandler* clientHandler) {
    if (logger::serverLogger) {
        logger::serverLogger->debug("RequestRouter handling message code: " + std::to_string(static_cast<int>(msg.code)));
    }

    // Execute Middlewares
    std::string errorMsg;
    for (const auto& middleware : middlewares) {
        if (!middleware->handle(clientFd, msg, errorMsg)) {
            if (logger::serverLogger) {
                logger::serverLogger->warn("Middleware rejected request from fd=" + std::to_string(clientFd) + ": " + errorMsg);
            }
            sendErrorResponse(clientFd, protocol::MsgCode::GENERAL_FAILURE, errorMsg);
            return;
        }
    }

    if (logger::serverLogger) {
        logger::serverLogger->debug("Middleware passed, routing to controller");
    }

    switch (msg.code) {
        // User / Auth
        case protocol::MsgCode::LOGIN_REQUEST:
            if (clientHandler) {
                if (logger::serverLogger) logger::serverLogger->debug("Routing to UserController::handleLoginRequest");
                userController->handleLoginRequest(clientFd, msg, clientHandler);
            } else {
                if (logger::serverLogger) logger::serverLogger->error("ClientHandler is null for LOGIN_REQUEST");
            }
            break;
        case protocol::MsgCode::LOGOUT_REQUEST:
            if (logger::serverLogger) logger::serverLogger->debug("Routing to UserController::handleLogoutRequest");
            userController->handleLogoutRequest(clientFd, msg);
            break;

        // Chat
        case protocol::MsgCode::SEND_CHAT_PRIVATE_REQUEST:
            chatController->handle_private_message(clientFd, msg);
            break;
        case protocol::MsgCode::CHAT_HISTORY_REQUEST:
            chatController->handle_chat_history(clientFd, msg);
            break;

        // Lesson
        case protocol::MsgCode::LESSON_LIST_REQUEST:
            lessonController->handleLessonListRequest(clientFd, msg);
            break;
        case protocol::MsgCode::STUDY_LESSON_REQUEST:
            lessonController->handleStudyLessonRequest(clientFd, msg);
            break;

        // Exercise
        case protocol::MsgCode::EXERCISE_LIST_REQUEST:
            exerciseController->handleExerciseListRequest(clientFd, msg);
            break;
        case protocol::MsgCode::MULTIPLE_CHOICE_REQUEST:
        case protocol::MsgCode::FILL_IN_REQUEST:
        case protocol::MsgCode::SENTENCE_ORDER_REQUEST:
        case protocol::MsgCode::REWRITE_SENTENCE_REQUEST:
        case protocol::MsgCode::WRITE_PARAGRAPH_REQUEST:
        case protocol::MsgCode::SPEAKING_TOPIC_REQUEST:
            exerciseController->handleSpecificExerciseRequest(clientFd, msg);
            break;
        case protocol::MsgCode::STUDY_EXERCISE_REQUEST:
            exerciseController->handleStudyExerciseRequest(clientFd, msg);
            break;

        // Submission
        case protocol::MsgCode::SUBMIT_ANSWER_REQUEST:
            submissionController->handleSubmission(clientFd, msg);
            break;
        case protocol::MsgCode::GRADE_SUBMISSION_REQUEST:
            submissionController->handleGradeSubmission(clientFd, msg);
            break;

        // Result
        case protocol::MsgCode::RESULT_LIST_REQUEST:
            resultController->handleDoneUndoneListRequest(clientFd, msg);
            break;
        case protocol::MsgCode::RESULT_DETAIL_REQUEST:
            resultController->handleResultDetailRequest(clientFd, msg);
            break;
        case protocol::MsgCode::RESULT_REQUEST: // This case is kept for handleResultRequest
            resultController->handleResultRequest(clientFd, msg);
            break;
        case protocol::MsgCode::PENDING_SUBMISSIONS_REQUEST:
            resultController->handlePendingSubmissionsRequest(clientFd, msg);
            break;

        // Exam
        case protocol::MsgCode::EXAM_LIST_REQUEST:
            examController->handleGetExams(clientFd, msg);
            break;
        case protocol::MsgCode::EXAM_REQUEST: // This case is kept for handleExamRequest
            examController->handleExamRequest(clientFd, msg);
            break;

        default:
            if (logger::serverLogger) {
                logger::serverLogger->warn("Unknown message code: " + std::to_string(static_cast<int>(msg.code)));
            }
            break;
    }
}

} // namespace server
