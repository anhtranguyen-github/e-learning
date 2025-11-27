#include "server/result_handler.h"
#include "common/payloads.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <vector>

namespace server {

// ============================================================================
// Constructor
// ============================================================================

ResultHandler::ResultHandler(std::shared_ptr<SessionManager> sm, std::shared_ptr<Database> db)
    : sessionManager(sm), db(db) {}

// ============================================================================
// Helper Functions
// ============================================================================

bool ResultHandler::sendMessage(int clientFd, const protocol::Message& msg) {
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

void ResultHandler::handleResultRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    logger::serverLogger->debug("Handling result request from fd=" + std::to_string(clientFd) + ", payload: " + payload);

    Payloads::ResultRequest req;
    req.deserialize(payload);

    std::string sessionToken = req.sessionToken;
    std::string targetType = req.targetType;
    int targetId = 0;
    try {
        targetId = std::stoi(req.targetId);
    } catch (...) {
        protocol::Message response(protocol::MsgCode::RESULT_LIST_FAILURE, "Invalid target ID");
        sendMessage(clientFd, response);
        return;
    }
    int userId = sessionManager->get_user_id_by_session(sessionToken);
    if (userId == -1) {
        protocol::Message response(protocol::MsgCode::RESULT_LIST_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }

    sessionManager->update_session(sessionToken);

    std::string query = "SELECT score, feedback FROM results WHERE user_id = " + std::to_string(userId) +
                        " AND target_type = '" + targetType + "' AND target_id = " + std::to_string(targetId);

    PGresult* result = db->query(query);
    std::string responsePayload;

    if (result && PQntuples(result) > 0) {
        std::string score = PQgetvalue(result, 0, 0);
        std::string feedback = PQgetvalue(result, 0, 1);
        protocol::Message response(protocol::MsgCode::RESULT_LIST_SUCCESS, responsePayload);
        sendMessage(clientFd, response);
    } else {
        protocol::Message response(protocol::MsgCode::RESULT_LIST_FAILURE, "Result not found");
        sendMessage(clientFd, response);
    }

    if (result) {
        PQclear(result);
    }
}

void ResultHandler::handleDoneUndoneListRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    logger::serverLogger->debug("Handling done/undone list request from fd=" + std::to_string(clientFd) + ", payload: " + payload);

    Payloads::ResultListRequest req;
    req.deserialize(payload);

    std::string sessionToken = req.sessionToken;
    std::string targetType = req.targetType;

    int userId = sessionManager->get_user_id_by_session(sessionToken);
    if (userId == -1) {
        protocol::Message response(protocol::MsgCode::RESULT_LIST_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }

    sessionManager->update_session(sessionToken);

    // This is a simplified implementation. A real implementation would need to join with the exercises/exams tables.
    std::string query = "SELECT target_id, score FROM results WHERE user_id = " + std::to_string(userId) +
                        " AND target_type = '" + targetType + "'";

    PGresult* result = db->query(query);
    std::string responsePayload;

    if (result) {
        for (int i = 0; i < PQntuples(result); ++i) {
            responsePayload += std::string(PQgetvalue(result, i, 0)) + "," + std::string(PQgetvalue(result, i, 1)) + ";";
        }
        protocol::Message response(protocol::MsgCode::RESULT_LIST_SUCCESS, responsePayload);
        sendMessage(clientFd, response);
    } else {
        protocol::Message response(protocol::MsgCode::RESULT_LIST_FAILURE, "Could not retrieve list");
        sendMessage(clientFd, response);
    }

    if (result) {
        PQclear(result);
    }
}

} // namespace server