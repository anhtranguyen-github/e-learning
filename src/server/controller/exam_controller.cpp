#include "server/controller/exam_controller.h"
#include "common/payloads.h"
#include "common/utils.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <sstream>
#include <vector>

namespace server {

ExamController::ExamController(std::shared_ptr<SessionManager> sessionMgr, 
                           std::shared_ptr<ExamRepository> examRepo,
                           std::shared_ptr<ResultRepository> resultRepo)
    : sessionManager(sessionMgr), examRepository(examRepo), resultRepository(resultRepo) {
}



void ExamController::handleStudentGetExams(int clientFd, const protocol::Message &msg) {
    std::string payload = msg.toString();

    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Handling EXAM_LIST_REQUEST from fd=" + std::to_string(clientFd) +
                                  ", payload: " + payload);
    }

    Payloads::ExamListRequest req;
    req.deserialize(payload);

    std::string sessionToken = req.sessionToken;
    std::string type = req.type;
    std::string level = req.level;
    int lessonId = req.lessonId.empty() ? -1 : std::stoi(req.lessonId);

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
            examList = examRepository->loadAllExams();
        } else {
            if (logger::serverLogger) {
                logger::serverLogger->debug("[DEBUG] Loading filtered exams");
            }
            examList = examRepository->loadExamsByFilter(lessonId, type, level);
        }

        int examCount = examList.count();
        if (logger::serverLogger) {
            logger::serverLogger->info("[INFO] Loaded " + std::to_string(examCount) + " exams");
        }

        // Serialize exam list for network transmission
        // Serialize exam list using DTOs
        std::vector<std::string> serializedDtos;
        for (const auto& exam : examList.getExams()) {
            serializedDtos.push_back(exam.toMetadataDTO().serialize());
        }
        std::string serializedList = std::to_string(serializedDtos.size());
        if (!serializedDtos.empty()) {
            serializedList += ";" + utils::join(serializedDtos, ';');
        }

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
void ExamController::handleStudentExamRequest(int clientFd, const protocol::Message &msg) {
    std::string payload = msg.toString();
    Payloads::ExamRequest req;
    req.deserialize(payload);

    if (!sessionManager->is_session_valid(req.sessionToken)) {
        protocol::Message response(protocol::MsgCode::EXAM_FAILURE, "Invalid session");
        sendMessage(clientFd, response);
        return;
    }
    sessionManager->update_session(req.sessionToken);

    int userId = sessionManager->get_user_id_by_session(req.sessionToken);
    if (userId == -1) {
        protocol::Message response(protocol::MsgCode::EXAM_FAILURE, "Invalid session");
        sendMessage(clientFd, response);
        return;
    }

    int examId;
    try {
        examId = std::stoi(req.examId);
    } catch (...) {
        std::string errorMsg = "Invalid exam ID format in EXAM_REQUEST from fd=" + std::to_string(clientFd) + ", ID: " + req.examId;
        if (logger::serverLogger) {
            logger::serverLogger->warn("[WARN] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::EXAM_FAILURE, "Invalid exam ID");
        sendMessage(clientFd, response);
        return;
    }

    if (logger::serverLogger) {
        logger::serverLogger->info("[INFO] ExamController: Handling exam request for ID: " + std::to_string(examId) + " from fd=" + std::to_string(clientFd));
    }

    // Check if exam already taken - only applies to STUDENTS
    // Teachers can view any exam for grading purposes
    std::string userRole = sessionManager->get_user_role_by_fd(clientFd);
    
    if (userRole == "student" && resultRepository->hasResult(userId, "exam", examId)) {
        std::string errorMsg = "Exam already taken by user " + std::to_string(userId);
        if (logger::serverLogger) {
            logger::serverLogger->info("[INFO] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::EXAM_ALREADY_TAKEN, "Exam already taken");
        sendMessage(clientFd, response);
        return;
    }

    Exam exam = examRepository->loadExamById(examId);
    
    if (exam.getExamId() == -1) { // Assuming -1 indicates exam not found
        std::string errorMsg = "Exam not found for ID: " + std::to_string(examId) + " requested by fd=" + std::to_string(clientFd);
        if (logger::serverLogger) {
            logger::serverLogger->warn("[WARN] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::EXAM_FAILURE, "Exam not found");
        sendMessage(clientFd, response);
        return;
    }

    if (logger::serverLogger) {
        logger::serverLogger->info("[INFO] ExamController: Found exam ID " + std::to_string(examId) + ", serializing for fd=" + std::to_string(clientFd));
    }

    Payloads::ExamDTO dto = exam.toDTO();
    protocol::Message response(protocol::MsgCode::EXAM_SUCCESS, dto.serialize());
    sendMessage(clientFd, response);
}

bool ExamController::sendMessage(int clientFd, const protocol::Message& msg) {
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