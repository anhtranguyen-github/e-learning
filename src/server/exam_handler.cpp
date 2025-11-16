#include "server/exam_handler.h"
#include "common/logger.h"
#include <json/json.h>
#include "common/utils.h"

namespace server {

ExamHandler::ExamHandler(std::shared_ptr<SessionManager> sm, std::shared_ptr<ExamLoader> el)
    : sessionManager(sm), examLoader(el) {}

void ExamHandler::handleGetExams(int clientFd, const protocol::Message &msg) {
    std::string payload = msg.toString();

    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Handling EXAM_LIST_REQUEST from fd=" + std::to_string(clientFd) +
                                  ", payload: " + payload);
    }

    // Parse payload: <session_token>[;<type>;<level>;<lesson_id>]
    std::vector<std::string> parts;
    std::istringstream iss(payload);
    std::string part;

    while (std::getline(iss, part, ';')) {
        parts.push_back(part);
    }

    if (parts.empty()) {
        std::string errorMsg = "Empty payload in EXAM_LIST_REQUEST from fd=" + std::to_string(clientFd);
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::EXAM_LIST_FAILURE, errorMsg);
        sendMessage(clientFd, response);
        return;
    }

    std::string sessionToken = parts[0];
    std::string type = parts.size() > 1 ? parts[1] : "";
    std::string level = parts.size() > 2 ? parts[2] : "";
    int lessonId = parts.size() > 3 ? std::stoi(parts[3]) : -1;

    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Parsed request - token: " + sessionToken +
                                  ", type: " + (type.empty() ? "<none>" : type) +
                                  ", level: " + (level.empty() ? "<none>" : level) +
                                  ", lessonId: " + (lessonId == -1 ? "<none>" : std::to_string(lessonId)));
    }

    // Validate session token
    if (!sessionManager->is_session_valid(sessionToken)) {
        std::string errorMsg = "Invalid or expired session token in EXAM_LIST_REQUEST from fd=" +
                             std::to_string(clientFd);
        if (logger::serverLogger) {
            logger::serverLogger->warn("[WARN] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::EXAM_LIST_FAILURE, errorMsg);
        sendMessage(clientFd, response);
        return;
    }

    // Update session activity
    sessionManager->update_session(sessionToken);

    // Load exams from database
    ExamList examList;

    try {
        if (type.empty() && level.empty() && lessonId == -1) {
            if (logger::serverLogger) {
                logger::serverLogger->debug("[DEBUG] Loading all exams");
            }
            examList = examLoader->loadAllExams();
        } else {
            if (logger::serverLogger) {
                logger::serverLogger->debug("[DEBUG] Loading filtered exams");
            }
            examList = examLoader->loadExamsByFilter(type, level, lessonId);
        }

        int examCount = examList.count();
        if (logger::serverLogger) {
            logger::serverLogger->info("[INFO] Loaded " + std::to_string(examCount) + " exams");
        }

        // Serialize exam list for network transmission
        std::string serializedList = examList.serializeForNetwork();

        // Send success response
        protocol::Message response(protocol::MsgCode::EXAM_LIST_SUCCESS, serializedList);

        if (sendMessage(clientFd, response)) {
            if (logger::serverLogger) {
                logger::serverLogger->info("[INFO] Successfully sent " + std::to_string(examCount) +
                                         " exams to fd=" + std::to_string(clientFd));
            }
        } else {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Failed to send exam list to fd=" +
                                          std::to_string(clientFd));
            }
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Exception in handleGetExams: " + std::string(e.what());
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::EXAM_LIST_FAILURE, errorMsg);
        sendMessage(clientFd, response);
    }
}
void ExamHandler::handleExamRequest(int clientFd, const protocol::Message &msg) {
    (void)clientFd; // Suppress unused parameter warning
    (void)msg;      // Suppress unused parameter warning
    // Placeholder implementation
    if (logger::serverLogger) {
        logger::serverLogger->info("Handling exam request");
    }
}

bool ExamHandler::sendMessage(int clientFd, const protocol::Message& msg) {
    std::vector<uint8_t> data = msg.serialize();
    
    ssize_t sent = send(clientFd, data.data(), data.size(), 0);
    
    if (sent < 0) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to send message to fd=" + std::to_string(clientFd));
        }
        return false;
    }
    
    return true;
}

} // namespace server