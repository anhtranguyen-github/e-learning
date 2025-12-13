#include "server/controller/feedback_controller.h"
#include "common/payloads.h"
#include "common/utils.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <sstream>
#include <vector>

namespace server {

FeedbackController::FeedbackController(std::shared_ptr<SessionManager> sessionMgr, 
                                       std::shared_ptr<ResultRepository> resultRepo,
                                       std::shared_ptr<ExerciseRepository> exerciseRepo,
                                       std::shared_ptr<ExamRepository> examRepo)
    : sessionManager(sessionMgr), resultRepo(resultRepo), exerciseRepo(exerciseRepo), examRepo(examRepo) {
}

bool FeedbackController::sendMessage(int clientFd, const protocol::Message& msg) {
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

void FeedbackController::handleGetSubmissions(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[FeedbackController] Getting submissions for fd=" + std::to_string(clientFd));
    }

    Payloads::PendingSubmissionsRequest req;
    req.deserialize(payload);

    if (!sessionManager->is_session_valid(req.sessionToken)) {
        protocol::Message response(protocol::MsgCode::PENDING_SUBMISSIONS_FAILURE, "Invalid session");
        sendMessage(clientFd, response);
        return;
    }

    sessionManager->update_session(req.sessionToken);

    // Get all submissions (both pending and graded)
    auto submissions = resultRepo->getSubmissions();

    // Serialize using SubmissionDTO
    std::vector<std::string> serializedDtos;
    for (const auto& sub : submissions) {
        serializedDtos.push_back(sub.serialize());
    }

    std::string responsePayload = std::to_string(serializedDtos.size());
    if (!serializedDtos.empty()) {
        responsePayload += ";" + utils::join(serializedDtos, ';');
    }

    protocol::Message response(protocol::MsgCode::PENDING_SUBMISSIONS_SUCCESS, responsePayload);
    sendMessage(clientFd, response);

    if (logger::serverLogger) {
        logger::serverLogger->info("[FeedbackController] Sent " + std::to_string(submissions.size()) + " submissions to fd=" + std::to_string(clientFd));
    }
}

void FeedbackController::handleGradeSubmission(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[FeedbackController] Handling grade submission from fd=" + std::to_string(clientFd));
    }

    Payloads::GradeSubmissionRequest req;
    req.deserialize(payload);

    int userId = sessionManager->get_user_id_by_session(req.sessionToken);
    if (userId == -1) {
        protocol::Message response(protocol::MsgCode::GRADE_SUBMISSION_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }

    sessionManager->update_session(req.sessionToken);

    int resultId = 0;
    double score = 0.0;
    try {
        resultId = std::stoi(req.resultId);
        score = std::stod(req.score);
    } catch (...) {
        protocol::Message response(protocol::MsgCode::GRADE_SUBMISSION_FAILURE, "Invalid result ID or score");
        sendMessage(clientFd, response);
        return;
    }

    if (resultRepo->updateResult(resultId, score, req.feedback, "graded", req.gradingDetails)) {
        protocol::Message response(protocol::MsgCode::GRADE_SUBMISSION_SUCCESS, "Grade updated successfully");
        sendMessage(clientFd, response);
        
        if (logger::serverLogger) {
            logger::serverLogger->info("[FeedbackController] Grade submitted for result ID " + req.resultId);
        }
    } else {
        protocol::Message response(protocol::MsgCode::GRADE_SUBMISSION_FAILURE, "Failed to update grade");
        sendMessage(clientFd, response);
    }
}

void FeedbackController::handleAddFeedback(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[FeedbackController] Adding feedback from fd=" + std::to_string(clientFd));
    }

    Payloads::AddFeedbackRequest req;
    req.deserialize(payload);

    int userId = sessionManager->get_user_id_by_session(req.sessionToken);
    if (userId == -1) {
        protocol::Message response(protocol::MsgCode::GRADE_SUBMISSION_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }

    sessionManager->update_session(req.sessionToken);

    int resultId = 0;
    try {
        resultId = std::stoi(req.resultId);
    } catch (...) {
        protocol::Message response(protocol::MsgCode::GRADE_SUBMISSION_FAILURE, "Invalid result ID");
        sendMessage(clientFd, response);
        return;
    }

    // Build feedback string - combine text and audio info
    std::string feedbackContent = req.feedbackText;
    if (req.feedbackType == "audio" && !req.audioData.empty()) {
        feedbackContent = "[AUDIO]" + req.audioData;
    }

    // Use updateResult to add feedback (keeping existing score)
    if (resultRepo->addFeedback(resultId, feedbackContent, req.feedbackType)) {
        protocol::Message response(protocol::MsgCode::GRADE_SUBMISSION_SUCCESS, "Feedback added successfully");
        sendMessage(clientFd, response);
        
        if (logger::serverLogger) {
            logger::serverLogger->info("[FeedbackController] Feedback added for result ID " + req.resultId);
        }
    } else {
        protocol::Message response(protocol::MsgCode::GRADE_SUBMISSION_FAILURE, "Failed to add feedback");
        sendMessage(clientFd, response);
    }
}

} // namespace server
