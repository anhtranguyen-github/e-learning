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
                // Objective types
                std::vector<std::string> userAnswers = utils::split(userAnswer, '^');
                std::vector<Question> questions = exercise.getQuestions();
                
                int correctCount = 0;
                size_t totalQuestions = questions.size();
                
                if (totalQuestions > 0) {
                    for (size_t i = 0; i < totalQuestions; ++i) {
                        std::string correct = questions[i].getAnswer();
                        std::string user = (i < userAnswers.size()) ? userAnswers[i] : "";
                        
                        if (user == correct) {
                            correctCount++;
                        }
                    }
                    score = (static_cast<double>(correctCount) / totalQuestions) * 100.0;
                    feedback = "You got " + std::to_string(correctCount) + " out of " + std::to_string(totalQuestions) + " correct.";
                } else {
                    // Fallback for empty questions (shouldn't happen with valid data)
                    score = 0.0;
                    feedback = "No questions found.";
                }
            }
        }
    } else if (targetType == "exam") {
        Exam exam = examRepo->loadExamById(targetId);
        if (exam.getExamId() != -1) {
            std::vector<std::string> userAnswers = utils::split(userAnswer, '^');
            std::vector<Question> questions = exam.getQuestions();
            
            int correctCount = 0;
            
            for (size_t i = 0; i < questions.size(); ++i) {
                std::string correct = questions[i].getAnswer();
                std::string user = (i < userAnswers.size()) ? userAnswers[i] : "";
                
                // Simple string comparison (case-sensitive for now)
                if (user == correct) {
                    correctCount++;
                }
            }
            
            if (!questions.empty()) {
                score = (static_cast<double>(correctCount) / questions.size()) * 100.0;
            }
            
            status = "graded";
            feedback = "You got " + std::to_string(correctCount) + " out of " + std::to_string(questions.size()) + " correct.";
        }
    }

    std::string query = "INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status) VALUES (" +
                        std::to_string(userId) + ", '" + targetType + "', " + std::to_string(targetId) +
                        ", " + std::to_string(score) + ", '" + userAnswer + "', '" + feedback + "', '" + status + "')";

    bool success = db->execute(query);

    if (!success) {
        logger::serverLogger->error("Failed to insert submission into database");
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
}

} // namespace server