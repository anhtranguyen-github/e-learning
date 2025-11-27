#include "server/controller/submission_controller.h"
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

SubmissionController::SubmissionController(std::shared_ptr<SessionManager> sessionMgr, 
                                           std::shared_ptr<Database> database,
                                           std::shared_ptr<ExerciseRepository> exRepo,
                                           std::shared_ptr<ExamRepository> eRepo)
    : sessionManager(sessionMgr), db(database), exerciseRepo(exRepo), examRepo(eRepo) {}

// ============================================================================
// Helper Functions
// ============================================================================

bool SubmissionController::sendMessage(int clientFd, const protocol::Message& msg) {
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

void SubmissionController::handleSubmission(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    logger::serverLogger->debug("Handling submission from fd=" + std::to_string(clientFd) + ", payload: " + payload);

    Payloads::SubmitAnswerRequest req;
    req.deserialize(payload);

    std::string sessionToken = req.sessionToken;
    std::string targetType = req.targetType;
    int targetId = 0;
    try {
        targetId = std::stoi(req.targetId);
    } catch (...) {
        protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_FAILURE, "Invalid target ID");
        sendMessage(clientFd, response);
        return;
    }
    std::string userAnswer = req.answer;

    int userId = sessionManager->get_user_id_by_session(sessionToken);
    if (userId == -1) {
        protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }

    sessionManager->update_session(sessionToken);

    // Grading Logic
    std::string status = "graded";
    double score = 0.0;
    std::string feedback = "";

    if (targetType == "exercise") {
        Exercise exercise = exerciseRepo->loadExerciseById(targetId);
        if (exercise.getExerciseId() != -1) {
            std::string type = exercise.getType();
            // Subjective types need instructor review
            if (type == "rewrite_sentence" || type == "essay" || type == "speaking" || type == "write_paragraph") {
                status = "pending";
                score = 0.0; // Placeholder
                feedback = "Pending instructor review";
            } else {
                // Objective types (if any in exercises table)
                // Simple string comparison for now
                if (userAnswer == exercise.getAnswer()) {
                    score = 100.0;
                    feedback = "Correct!";
                } else {
                    score = 0.0;
                    feedback = "Incorrect. Correct answer: " + exercise.getAnswer();
                }
            }
        }
    } else if (targetType == "exam") {
        Exam exam = examRepo->loadExamById(targetId);
        if (exam.getExamId() != -1) {
            // Exams are objective but we currently lack answer key in Exam model
            // For now, we mark as graded but with 0 score, or we could mark pending if we want manual check
            // Assuming auto-grading is required:
            status = "graded";
            score = 0.0; // Cannot verify without answer key
            feedback = "Exam submitted.";
        }
    }

    std::string query = "INSERT INTO results (user_id, target_type, target_id, score, feedback, status) VALUES (" +
                        std::to_string(userId) + ", '" + targetType + "', " + std::to_string(targetId) +
                        ", " + std::to_string(score) + ", '" + userAnswer + "', '" + status + "')";

    PGresult* result = db->query(query);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        logger::serverLogger->error("Failed to insert submission into database: " + std::string(PQresultErrorMessage(result)));
        Payloads::GenericResponse resp;
        resp.success = false;
        resp.message = "Failed to save submission";
        protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_FAILURE, resp.serialize());
        sendMessage(clientFd, response);
    } else {
        Payloads::GenericResponse resp;
        resp.success = true;
        resp.message = feedback; // Send feedback to client
        protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_SUCCESS, resp.serialize());
        sendMessage(clientFd, response);
    }

    if (result) {
        PQclear(result);
    }
}

} // namespace server