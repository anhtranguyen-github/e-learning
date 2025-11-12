#include "server/submission_handler.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <vector>

namespace server {

// ============================================================================
// Constructor
// ============================================================================

SubmissionHandler::SubmissionHandler(std::shared_ptr<SessionManager> sm, std::shared_ptr<Database> db)
    : sessionManager(sm), db(db) {}

// ============================================================================
// Helper Functions
// ============================================================================

bool SubmissionHandler::sendMessage(int clientFd, const protocol::Message& msg) {
    try {
        std::vector<uint8_t> serialized = msg.serialize();
        ssize_t bytesSent = send(clientFd, serialized.data(), serialized.size(), 0);
        
        if (bytesSent < 0) {
            logger::serverLogger->error("Failed to send message to fd=" + std::to_string(clientFd));
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        logger::serverLogger->error("Exception in sendMessage: " + std::string(e.what()));
        return false;
    }
}

// ============================================================================
// Message Handlers
// ============================================================================

void SubmissionHandler::handleSubmission(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    logger::serverLogger->debug("Handling submission from fd=" + std::to_string(clientFd) + ", payload: " + payload);

    // Payload: <token>;<target_type>;<target_id>;<user_answer>
    std::vector<std::string> parts;
    std::istringstream iss(payload);
    std::string part;
    while (std::getline(iss, part, ';')) {
        parts.push_back(part);
    }

    if (parts.size() < 4) {
        // Assuming a generic failure message for now
        protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_FAILURE, "Invalid submission format");
        sendMessage(clientFd, response);
        return;
    }

    std::string sessionToken = parts[0];
    std::string targetType = parts[1];
    int targetId = std::stoi(parts[2]);
    std::string userAnswer = parts[3];

    auto session = sessionManager->getSession(sessionToken);
    if (!session) {
        protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }

    sessionManager->updateLastActive(sessionToken);
    int userId = session->userId;

    std::string query = "INSERT INTO results (user_id, target_type, target_id, feedback) VALUES (" +
                        std::to_string(userId) + ", '" + targetType + "', " + std::to_string(targetId) +
                        ", '" + userAnswer + "')";

    PGresult* result = db->query(query);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        logger::serverLogger->error("Failed to insert submission into database: " + std::string(PQresultErrorMessage(result)));
        protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_FAILURE, "Failed to save submission");
        sendMessage(clientFd, response);
    } else {
        protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_SUCCESS, "Submission received");
        sendMessage(clientFd, response);
    }

    if (result) {
        PQclear(result);
    }
}

} // namespace server