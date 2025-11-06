#ifndef EXAM_HANDLER_H
#define EXAM_HANDLER_H

#include "common/protocol.h"
#include "server/exam_loader.h"
#include "server/session.h"

#include "server/session.h"
#include <memory>

#include <sys/socket.h>

namespace server
{
    class ExamHandler
    {
    public:
        ExamHandler(std::shared_ptr<SessionManager> sm, std::shared_ptr<ExamLoader> el);
        void handleGetExams(int clientFd, const protocol::Message &msg);
        void handleExamRequest(int clientFd, const protocol::Message &msg);

    private:
        bool sendMessage(int clientFd, const protocol::Message& msg);
        std::shared_ptr<SessionManager> sessionManager;
        std::shared_ptr<ExamLoader> examLoader;
    };
}
#endif // EXAM_HANDLER_H
