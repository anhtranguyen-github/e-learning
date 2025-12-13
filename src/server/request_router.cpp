#include "server/request_router.h"
#include "server/controller/user_controller.h"
#include "server/controller/chat_controller.h"
#include "server/controller/lesson_controller.h"
#include "server/controller/exercise_controller.h"
#include "server/controller/submission_controller.h"
#include "server/controller/result_controller.h"
#include "server/controller/student_exam_controller.h"
#include "server/controller/teacher_exam_controller.h"
#include "server/controller/feedback_controller.h"
#include "server/repository/user_repository.h"
#include "server/repository/lesson_repository.h"
#include "server/repository/exercise_repository.h"
#include "server/repository/exam_repository.h"
#include "server/repository/game_repository.h"
#include "server/controller/game_controller.h"
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
    auto chatRepo = std::make_shared<ChatRepository>(db);
    auto gameRepo = std::make_shared<GameRepository>(db);

    // Initialize Controllers
    userController = std::make_shared<UserController>(userRepo, sessionManager, connectionManager);
    chatController = std::make_shared<ChatController>(chatRepo, userRepo, connectionManager, sessionManager);
    lessonController = std::make_shared<LessonController>(sessionManager, lessonRepo);
    exerciseController = std::make_shared<ExerciseController>(sessionManager, exerciseRepo);
    submissionController = std::make_shared<SubmissionController>(sessionManager, resultRepo, exerciseRepo, examRepo);
    resultController = std::make_shared<ResultController>(sessionManager, resultRepo);
    studentExamController = std::make_shared<StudentExamController>(sessionManager, examRepo, resultRepo);
    teacherExamController = std::make_shared<TeacherExamController>(sessionManager, examRepo);
    feedbackController = std::make_shared<FeedbackController>(sessionManager, resultRepo, exerciseRepo, examRepo);
    gameController = std::make_shared<GameController>(sessionManager, gameRepo, resultRepo);

    // Register Default Middlewares
    registerMiddleware(std::make_shared<LoggingMiddleware>());
    registerMiddleware(std::make_shared<AuthMiddleware>(sessionManager));
    registerMiddleware(std::make_shared<RBACMiddleware>(sessionManager));
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
                if (logger::serverLogger) logger::serverLogger->debug("Routing to UserController::handleUserLoginRequest");
                userController->handleUserLoginRequest(clientFd, msg, clientHandler);
            } else {
                if (logger::serverLogger) logger::serverLogger->error("ClientHandler is null for LOGIN_REQUEST");
            }
            break;
        case protocol::MsgCode::LOGOUT_REQUEST:
            if (logger::serverLogger) logger::serverLogger->debug("Routing to UserController::handleUserLogoutRequest");
            userController->handleUserLogoutRequest(clientFd, msg);
            break;
        case protocol::MsgCode::REGISTER_REQUEST:
            if (logger::serverLogger) logger::serverLogger->debug("Routing to UserController::handleUserRegisterRequest");
            userController->handleUserRegisterRequest(clientFd, msg);
            break;

        // Chat
        case protocol::MsgCode::SEND_CHAT_PRIVATE_REQUEST:
            chatController->handleUserSendPrivateMessage(clientFd, msg);
            break;
        case protocol::MsgCode::CHAT_HISTORY_REQUEST:
            chatController->handleUserGetChatHistory(clientFd, msg);
            break;
        case protocol::MsgCode::RECENT_CHATS_REQUEST:
            chatController->handleUserGetRecentChats(clientFd, msg);
            break;

        // Voice Calls
        case protocol::MsgCode::CALL_INITIATE_REQUEST:
            chatController->handleCallInitiate(clientFd, msg);
            break;
        case protocol::MsgCode::CALL_ANSWER_REQUEST:
            chatController->handleCallAnswer(clientFd, msg);
            break;
        case protocol::MsgCode::CALL_DECLINE_REQUEST:
            chatController->handleCallDecline(clientFd, msg);
            break;
        case protocol::MsgCode::CALL_END_REQUEST:
            chatController->handleCallEnd(clientFd, msg);
            break;

        // Lesson
        case protocol::MsgCode::LESSON_LIST_REQUEST:
            lessonController->handleUserLessonListRequest(clientFd, msg);
            break;
        case protocol::MsgCode::STUDY_LESSON_REQUEST:
            lessonController->handleUserStudyLessonRequest(clientFd, msg);
            break;

        // Exercise
        case protocol::MsgCode::EXERCISE_LIST_REQUEST:
            exerciseController->handleStudentExerciseListRequest(clientFd, msg);
            break;
        case protocol::MsgCode::MULTIPLE_CHOICE_REQUEST:
        case protocol::MsgCode::FILL_IN_REQUEST:
        case protocol::MsgCode::SENTENCE_ORDER_REQUEST:
        case protocol::MsgCode::REWRITE_SENTENCE_REQUEST:
        case protocol::MsgCode::WRITE_PARAGRAPH_REQUEST:
        case protocol::MsgCode::SPEAKING_TOPIC_REQUEST:
            exerciseController->handleStudentSpecificExerciseRequest(clientFd, msg);
            break;
        case protocol::MsgCode::STUDY_EXERCISE_REQUEST:
            exerciseController->handleStudentStudyExerciseRequest(clientFd, msg);
            break;

        // Submission
        case protocol::MsgCode::SUBMIT_ANSWER_REQUEST:
            submissionController->handleStudentSubmission(clientFd, msg);
            break;
        case protocol::MsgCode::GRADE_SUBMISSION_REQUEST:
            feedbackController->handleGradeSubmission(clientFd, msg);
            break;

        // Result
        case protocol::MsgCode::RESULT_LIST_REQUEST:
            resultController->handleStudentResultListRequest(clientFd, msg);
            break;
        case protocol::MsgCode::RESULT_DETAIL_REQUEST:
            resultController->handleStudentResultDetailRequest(clientFd, msg);
            break;
        case protocol::MsgCode::RESULT_REQUEST: // This case is kept for handleResultRequest
            resultController->handleStudentResultRequest(clientFd, msg);
            break;
        case protocol::MsgCode::PENDING_SUBMISSIONS_REQUEST:
            feedbackController->handleGetSubmissions(clientFd, msg);
            break;

        // Exam (Student)
        case protocol::MsgCode::EXAM_LIST_REQUEST:
            studentExamController->handleGetExams(clientFd, msg);
            break;
        case protocol::MsgCode::EXAM_REQUEST:
            studentExamController->handleExamRequest(clientFd, msg);
            break;

        // Exam (Teacher)
        case protocol::MsgCode::EXAM_REVIEW_REQUEST:
            teacherExamController->handleExamReview(clientFd, msg);
            break;

        // Games
        case protocol::MsgCode::GAME_LIST_REQUEST:
            gameController->handleGameListRequest(clientFd, msg);
            break;
        case protocol::MsgCode::GAME_LEVEL_LIST_REQUEST:
            gameController->handleGameLevelListRequest(clientFd, msg);
            break;
        case protocol::MsgCode::GAME_DATA_REQUEST:
            gameController->handleGameDataRequest(clientFd, msg);
            break;
        case protocol::MsgCode::GAME_SUBMIT_REQUEST:
            gameController->handleGameSubmitRequest(clientFd, msg);
            break;

        default:
            if (logger::serverLogger) {
                logger::serverLogger->warn("Unknown message code: " + std::to_string(static_cast<int>(msg.code)));
            }
            break;
    }
}

void RequestRouter::processTimeouts() {
    if (chatController) {
        chatController->processCallTimeouts();
    }
}

} // namespace server
