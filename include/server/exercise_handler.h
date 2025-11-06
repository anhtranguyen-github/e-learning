#ifndef EXERCISE_HANDLER_H
#define EXERCISE_HANDLER_H

#include "server/session.h"
#include "server/exercise_loader.h"
#include "common/protocol.h"
#include <memory>

namespace server {

class ExerciseHandler {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ExerciseLoader> exerciseLoader;

    bool sendMessage(int clientFd, const protocol::Message& msg);

public:
    ExerciseHandler(std::shared_ptr<SessionManager> sm, std::shared_ptr<ExerciseLoader> el);

    void handleExerciseRequest(int clientFd, const protocol::Message& msg);
    void handleExamRequest(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // EXERCISE_HANDLER_H