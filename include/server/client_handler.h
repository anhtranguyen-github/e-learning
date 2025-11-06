#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/user_manager.h"
#include "server/lesson_handler.h"
#include "server/exercise_handler.h"
#include "server/submission_handler.h"
#include "server/result_handler.h"
#include <vector>
#include <memory>

namespace server {

class ClientHandler {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<UserManager> userManager;
    std::shared_ptr<LessonHandler> lessonHandler;
    std::shared_ptr<ExerciseHandler> exerciseHandler;
    std::shared_ptr<SubmissionHandler> submissionHandler;
    std::shared_ptr<ResultHandler> resultHandler;

    // Handle specific message types
    void handleLoginRequest(int clientFd, const protocol::Message& msg);
    void handleLogoutRequest(int clientFd, const protocol::Message& msg);
    void handleHeartbeat(int clientFd, const protocol::Message& msg);
    void handleDisconnectRequest(int clientFd);

    // Send message to client
    bool sendMessage(int clientFd, const protocol::Message& msg);
    
    // Receive message from client
    protocol::Message receiveMessage(int clientFd);

public:
    ClientHandler(std::shared_ptr<SessionManager> sm, 
                 std::shared_ptr<UserManager> um,
                 std::shared_ptr<LessonHandler> lh,
                 std::shared_ptr<ExerciseHandler> eh,
                 std::shared_ptr<SubmissionHandler> sh,
                 std::shared_ptr<ResultHandler> rh);

    // Set lesson handler (optional, for lesson features)
    void setLessonHandler(std::shared_ptr<LessonHandler> lh) { lessonHandler = lh; }

    // Process incoming message from client
    void processMessage(int clientFd, const std::vector<uint8_t>& data);

    // Handle client disconnect
    void handleClientDisconnect(int clientFd);
};

} // namespace server

#endif // CLIENT_HANDLER_H
