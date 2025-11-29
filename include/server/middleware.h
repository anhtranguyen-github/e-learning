#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include "common/protocol.h"
#include "common/logger.h"
#include "server/session.h"
#include <string>
#include <memory>
#include <iostream>

namespace server {

class Middleware {
public:
    virtual ~Middleware() = default;
    // Returns true if processing should continue, false if request should be rejected
    virtual bool handle(int clientFd, const protocol::Message& msg, std::string& errorMsg) = 0;
};

class LoggingMiddleware : public Middleware {
public:
    bool handle(int clientFd, const protocol::Message& msg, std::string& /*errorMsg*/) override {
        if (logger::serverLogger) {
            logger::serverLogger->info("Request: Code=" + std::to_string(static_cast<int>(msg.code)) + 
                                      ", Size=" + std::to_string(msg.serialize().size()) + 
                                      ", Fd=" + std::to_string(clientFd));
        }
        return true;
    }
};

class AuthMiddleware : public Middleware {
private:
    std::shared_ptr<SessionManager> sessionManager;

public:
    AuthMiddleware(std::shared_ptr<SessionManager> sm) : sessionManager(sm) {}

    bool handle(int clientFd, const protocol::Message& msg, std::string& errorMsg) override {
        // Skip auth check for Login/Register/Disconnect/Heartbeat
        if (msg.code == protocol::MsgCode::LOGIN_REQUEST || 
            msg.code == protocol::MsgCode::DISCONNECT_REQUEST ||
            msg.code == protocol::MsgCode::HEARTBEAT) {
            return true;
        }

        // For other requests, extract session token
        // This is tricky because the session token location varies by message type.
        // However, most requests (except Login) should probably include it or we rely on the fact that
        // the client MUST be logged in.
        
        // A better approach for AuthMiddleware in this specific protocol:
        // Check if the clientFd maps to a valid session in SessionManager.
        // SessionManager::get_user_id_by_fd(clientFd) returns -1 if not logged in.
        
        int userId = sessionManager->get_user_id_by_fd(clientFd);
        if (userId == -1) {
            errorMsg = "Unauthorized: Login required";
            return false;
        }
        
        return true;
    }
};

class RBACMiddleware : public Middleware {
private:
    std::shared_ptr<SessionManager> sessionManager;

public:
    RBACMiddleware(std::shared_ptr<SessionManager> sm) : sessionManager(sm) {}

    bool handle(int clientFd, const protocol::Message& msg, std::string& errorMsg) override {
        std::string role = sessionManager->get_user_role_by_fd(clientFd);

        // Teacher/Admin only requests
        if (msg.code == protocol::MsgCode::GRADE_SUBMISSION_REQUEST ||
            msg.code == protocol::MsgCode::PENDING_SUBMISSIONS_REQUEST) {
            
            if (role != "teacher" && role != "admin") {
                errorMsg = "Unauthorized: Teacher or Admin role required";
                return false;
            }
        }
        
        // Student Role Handling:
        // Students are implicitly handled here. They are blocked from the Teacher/Admin requests above.
        // They are allowed to proceed to the Student-only requests below (which are not blocked for them).
        // If we needed to restrict students from specific "Admin-only" features, we would add a check here.

        // Student only requests (Teachers cannot access these)
        // Teacher: view lesson, view pending submited exam/excercises, chat.
        if (role == "teacher") {
            if (msg.code == protocol::MsgCode::EXERCISE_LIST_REQUEST ||
                msg.code == protocol::MsgCode::STUDY_EXERCISE_REQUEST ||
                msg.code == protocol::MsgCode::MULTIPLE_CHOICE_REQUEST ||
                msg.code == protocol::MsgCode::FILL_IN_REQUEST ||
                msg.code == protocol::MsgCode::SENTENCE_ORDER_REQUEST ||
                msg.code == protocol::MsgCode::REWRITE_SENTENCE_REQUEST ||
                msg.code == protocol::MsgCode::WRITE_PARAGRAPH_REQUEST ||
                msg.code == protocol::MsgCode::SPEAKING_TOPIC_REQUEST ||
                msg.code == protocol::MsgCode::SUBMIT_ANSWER_REQUEST ||
                msg.code == protocol::MsgCode::RESULT_LIST_REQUEST ||
                msg.code == protocol::MsgCode::RESULT_DETAIL_REQUEST ||
                msg.code == protocol::MsgCode::RESULT_REQUEST ||
                msg.code == protocol::MsgCode::EXAM_LIST_REQUEST ||
                msg.code == protocol::MsgCode::EXAM_REQUEST) {
                
                errorMsg = "Unauthorized: Teachers cannot perform student actions (Exercises, Exams, Results)";
                return false;
            }
        }

        return true;
    }
};

} // namespace server

#endif // MIDDLEWARE_H
