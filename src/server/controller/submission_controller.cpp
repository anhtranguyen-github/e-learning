#include "server/controller/submission_controller.h"
#include "common/payloads.h"
#include "common/utils.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <algorithm>

namespace server {

// ============================================================================
// Constructor
// ============================================================================

SubmissionController::SubmissionController(std::shared_ptr<SessionManager> sessionMgr, 
                                           std::shared_ptr<ResultRepository> resultRepo,
                                           std::shared_ptr<ExerciseRepository> exerciseRepo,
                                           std::shared_ptr<ExamRepository> examRepo)
    : sessionManager(sessionMgr), resultRepo(resultRepo), exerciseRepo(exerciseRepo), examRepo(examRepo) {
}

// ============================================================================
// Helper Functions
// ============================================================================

bool SubmissionController::sendMessage(int clientFd, const protocol::Message& msg) {
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

static std::string trimWhitespace(const std::string& value) {
    const char* whitespace = " \t\n\r";
    const auto start = value.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(whitespace);
    return value.substr(start, end - start + 1);
}

void SubmissionController::handleStudentSubmission(int clientFd, const protocol::Message& msg) {
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

    // Check if exam already taken (by the requesting user)
    if (targetType == "exam") {
        if (resultRepo->hasResult(userId, targetType, targetId)) {
            protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_FAILURE, "Exam already taken");
            sendMessage(clientFd, response);
            return;
        }
    }

    double score = 0.0;
    std::string feedback = "Submission received";
    std::string status = "graded";

    if (targetType == "exercise") {
        Exercise exercise = exerciseRepo->loadExerciseById(targetId);
        if (exercise.getExerciseId() != -1) {
            const std::string exerciseType = exercise.getType();
            if (exerciseType == "essay" || exerciseType == "speaking") {
                status = "pending";
                score = 0.0;
                feedback = "Pending instructor review";
            } else if (exerciseType == "rewrite_sentence") {
                std::string correct = trimWhitespace(exercise.getAnswer());
                std::string user = trimWhitespace(userAnswer);
                if (!correct.empty() && user == correct) {
                    score = 100.0;
                    feedback = "Correct.";
                } else {
                    score = 0.0;
                    feedback = "Incorrect.";
                }
            } else {
                std::vector<Question> questions = exercise.getQuestions();
                if (questions.empty()) {
                    score = 0.0;
                    feedback = "Error: No questions found for exercise";
                } else {
                    std::vector<std::string> userAnswers = utils::split(userAnswer, '^');
                    int correctCount = 0;

                    for (size_t i = 0; i < questions.size(); ++i) {
                        std::string correct = questions[i].getAnswer();
                        std::string user = (i < userAnswers.size()) ? userAnswers[i] : "";
                        
                        if (user == correct) {
                            correctCount++;
                        }
                    }

                    score = (static_cast<double>(correctCount) / questions.size()) * 100.0;
                    feedback = "You got " + std::to_string(correctCount) + " out of " + std::to_string(questions.size()) + " correct.";
                }
            }
        }
    } else if (targetType == "exam") {
        Exam exam = examRepo->loadExamById(targetId);
        if (exam.getExamId() != -1) {
            std::vector<std::string> userAnswers = utils::split(userAnswer, '^');
            std::vector<Question> questions = exam.getQuestions();
            
            int correctCount = 0;
            bool hasSubjective = false;

            for (size_t i = 0; i < questions.size(); ++i) {
                std::string qType = questions[i].getType();
                // Check for subjective types
                if (qType == "essay" || qType == "speaking" || qType == "rewrite_sentence" || qType == "write_paragraph") {
                    hasSubjective = true;
                }

                std::string correct = questions[i].getAnswer();
                std::string user = (i < userAnswers.size()) ? userAnswers[i] : "";
                
                // Simple string comparison (case-sensitive for now)
                if (user == correct) {
                    correctCount++;
                }
            }
            
            if (hasSubjective) {
                status = "pending";
                score = 0.0;
                feedback = "Pending instructor review";
            } else {
                if (!questions.empty()) {
                    score = (static_cast<double>(correctCount) / questions.size()) * 100.0;
                }
                status = "graded";
                feedback = "You got " + std::to_string(correctCount) + " out of " + std::to_string(questions.size()) + " correct.";
            }
        }
    }

    if (resultRepo->saveResult(userId, targetType, targetId, score, userAnswer, feedback, status)) {
        Payloads::ResultDTO resultDto;
        resultDto.score = std::to_string(score);
        resultDto.feedback = feedback;
        protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_SUCCESS, resultDto.serialize());
        sendMessage(clientFd, response);
    } else {
        protocol::Message response(protocol::MsgCode::SUBMIT_ANSWER_FAILURE, "Failed to save result");
        sendMessage(clientFd, response);
    }
}

void SubmissionController::handleTeacherGradeSubmission(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    logger::serverLogger->debug("Handling grade submission from fd=" + std::to_string(clientFd));

    Payloads::GradeSubmissionRequest req;
    req.deserialize(payload);

    // Verify admin/teacher role (simplified)
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
    } else {
        protocol::Message response(protocol::MsgCode::GRADE_SUBMISSION_FAILURE, "Failed to update grade");
        sendMessage(clientFd, response);
    }
}

} // namespace server
