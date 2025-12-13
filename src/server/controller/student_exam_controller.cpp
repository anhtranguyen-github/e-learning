#include "server/controller/student_exam_controller.h"
#include "common/payloads.h"
#include "common/utils.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <sstream>
#include <vector>

namespace server {

StudentExamController::StudentExamController(std::shared_ptr<SessionManager> sessionMgr, 
                                             std::shared_ptr<ExamRepository> examRepo,
                                             std::shared_ptr<ResultRepository> resultRepo)
    : sessionManager(sessionMgr), examRepository(examRepo), resultRepository(resultRepo) {
}

void StudentExamController::handleGetExams(int clientFd, const protocol::Message &msg) {
    std::string payload = msg.toString();

    if (logger::serverLogger) {
        logger::serverLogger->debug("[StudentExamController] Handling EXAM_LIST_REQUEST from fd=" + std::to_string(clientFd));
    }

    Payloads::ExamListRequest req;
    req.deserialize(payload);

    std::string sessionToken = req.sessionToken;
    std::string type = req.type;
    std::string level = req.level;
    int lessonId = req.lessonId.empty() ? -1 : std::stoi(req.lessonId);

    if (!sessionManager->is_session_valid(sessionToken)) {
        protocol::Message response(protocol::MsgCode::EXAM_LIST_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }

    sessionManager->update_session(sessionToken);

    ExamList examList;

    try {
        if (type.empty() && level.empty() && lessonId == -1) {
            examList = examRepository->loadAllExams();
        } else {
            examList = examRepository->loadExamsByFilter(lessonId, type, level);
        }

        std::vector<std::string> serializedDtos;
        for (const auto& exam : examList.getExams()) {
            serializedDtos.push_back(exam.toMetadataDTO().serialize());
        }
        std::string serializedList = std::to_string(serializedDtos.size());
        if (!serializedDtos.empty()) {
            serializedList += ";" + utils::join(serializedDtos, ';');
        }

        protocol::Message response(protocol::MsgCode::EXAM_LIST_SUCCESS, serializedList);
        sendMessage(clientFd, response);

        if (logger::serverLogger) {
            logger::serverLogger->info("[StudentExamController] Sent " + std::to_string(examList.count()) + " exams to fd=" + std::to_string(clientFd));
        }
    } catch (const std::exception& e) {
        protocol::Message response(protocol::MsgCode::EXAM_LIST_FAILURE, std::string("Error: ") + e.what());
        sendMessage(clientFd, response);
    }
}

void StudentExamController::handleExamRequest(int clientFd, const protocol::Message &msg) {
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
        protocol::Message response(protocol::MsgCode::EXAM_FAILURE, "Invalid exam ID");
        sendMessage(clientFd, response);
        return;
    }

    if (logger::serverLogger) {
        logger::serverLogger->info("[StudentExamController] Student requesting exam ID: " + std::to_string(examId));
    }

    // Students cannot retake exams they've already completed
    if (resultRepository->hasResult(userId, "exam", examId)) {
        if (logger::serverLogger) {
            logger::serverLogger->info("[StudentExamController] Exam already taken by student " + std::to_string(userId));
        }
        protocol::Message response(protocol::MsgCode::EXAM_ALREADY_TAKEN, "Exam already taken");
        sendMessage(clientFd, response);
        return;
    }

    Exam exam = examRepository->loadExamById(examId);
    
    if (exam.getExamId() == -1) {
        protocol::Message response(protocol::MsgCode::EXAM_FAILURE, "Exam not found");
        sendMessage(clientFd, response);
        return;
    }

    Payloads::ExamDTO dto = exam.toDTO();
    protocol::Message response(protocol::MsgCode::EXAM_SUCCESS, dto.serialize());
    sendMessage(clientFd, response);
}

bool StudentExamController::sendMessage(int clientFd, const protocol::Message& msg) {
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