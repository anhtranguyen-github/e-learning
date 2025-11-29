#include "server/controller/result_controller.h"
#include "common/payloads.h"
#include "common/utils.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <vector>

namespace server {

// ============================================================================
// Constructor
// ============================================================================

ResultController::ResultController(std::shared_ptr<SessionManager> sessionMgr, 
                               std::shared_ptr<ResultRepository> repo)
    : sessionManager(sessionMgr), resultRepo(repo) {
}

// ============================================================================
// Helper Functions
// ============================================================================

bool ResultController::sendMessage(int clientFd, const protocol::Message& msg) {
    try {
        std::vector<uint8_t> serialized = msg.serialize();
        ssize_t bytesSent = send(clientFd, serialized.data(), serialized.size(), 0);
        
        if (bytesSent < 0) {
            if (logger::serverLogger) {
                logger::serverLogger->error("Failed to send message to fd=" + std::to_string(clientFd));
            }
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

void ResultController::handleResultRequest(int clientFd, const protocol::Message& msg) {
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

    double score;
    std::string feedback;
    std::string status;

    if (resultRepo->getResult(userId, targetType, targetId, score, feedback, status)) {
        Payloads::ResultDTO dto;
        dto.score = std::to_string(score);
        dto.feedback = feedback;
        protocol::Message response(protocol::MsgCode::RESULT_LIST_SUCCESS, dto.serialize());
        sendMessage(clientFd, response);
    } else {
        protocol::Message response(protocol::MsgCode::RESULT_LIST_FAILURE, "Result not found");
        sendMessage(clientFd, response);
    }
}

void ResultController::handleDoneUndoneListRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    logger::serverLogger->debug("Handling done/undone list request from fd=" + std::to_string(clientFd) + ", payload: " + payload);

    logger::serverLogger->debug("Deserializing payload");
    Payloads::ResultListRequest req;
    req.deserialize(payload);

    std::string sessionToken = req.sessionToken;
    std::string targetType = req.targetType;
    logger::serverLogger->debug("SessionToken: " + sessionToken + ", TargetType: " + targetType);

    if (!sessionManager) {
        logger::serverLogger->error("SessionManager is null!");
        return;
    }

    logger::serverLogger->debug("Getting user ID");
    int userId = sessionManager->get_user_id_by_session(sessionToken);
    logger::serverLogger->debug("User ID: " + std::to_string(userId));

    if (userId == -1) {
        protocol::Message response(protocol::MsgCode::RESULT_LIST_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }

    logger::serverLogger->debug("Updating session");
    sessionManager->update_session(sessionToken);

    if (!resultRepo) {
        logger::serverLogger->error("ResultRepository is null in handleDoneUndoneListRequest");
        protocol::Message response(protocol::MsgCode::RESULT_LIST_FAILURE, "Internal server error");
        sendMessage(clientFd, response);
        return;
    }

    logger::serverLogger->debug("Calling resultRepo->getResultsByUser");
    std::vector<Payloads::ResultSummaryDTO> results = resultRepo->getResultsByUser(userId, targetType);
    logger::serverLogger->debug("resultRepo->getResultsByUser returned " + std::to_string(results.size()) + " results");
    
    std::vector<std::string> serializedDtos;
    for (const auto& dto : results) {
        serializedDtos.push_back(dto.serialize());
    }
    
    std::string responsePayload = std::to_string(serializedDtos.size());
    if (!serializedDtos.empty()) {
            responsePayload += ";" + utils::join(serializedDtos, ';');
    }
    protocol::Message response(protocol::MsgCode::RESULT_LIST_SUCCESS, responsePayload);
    sendMessage(clientFd, response);
}

void ResultController::handlePendingSubmissionsRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    logger::serverLogger->debug("Handling pending submissions request from fd=" + std::to_string(clientFd));

    Payloads::PendingSubmissionsRequest req;
    req.deserialize(payload);

    // Verify admin/teacher role (simplified: just check valid session for now, ideally check role)
    int userId = sessionManager->get_user_id_by_session(req.sessionToken);
    if (userId == -1) {
        protocol::Message response(protocol::MsgCode::RESULT_LIST_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }

    sessionManager->update_session(req.sessionToken);

    std::vector<Payloads::PendingSubmissionDTO> submissions = resultRepo->getPendingSubmissions();
    
    std::vector<std::string> serializedDtos;
    for (const auto& dto : submissions) {
        serializedDtos.push_back(dto.serialize());
    }
    
    std::string responsePayload = std::to_string(serializedDtos.size());
    if (!serializedDtos.empty()) {
            responsePayload += ";" + utils::join(serializedDtos, ';');
    }
    // Reusing RESULT_LIST_SUCCESS for now, or create a new MsgCode
    protocol::Message response(protocol::MsgCode::PENDING_SUBMISSIONS_SUCCESS, responsePayload);
    sendMessage(clientFd, response);
}

} // namespace server