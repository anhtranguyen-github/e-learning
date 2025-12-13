#include "server/controller/teacher_exam_controller.h"
#include "common/payloads.h"
#include "common/utils.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <vector>

namespace server {

TeacherExamController::TeacherExamController(std::shared_ptr<SessionManager> sessionMgr, 
                                             std::shared_ptr<ExamRepository> examRepo)
    : sessionManager(sessionMgr), examRepository(examRepo) {
}

void TeacherExamController::handleExamReview(int clientFd, const protocol::Message &msg) {
    std::string payload = msg.toString();
    Payloads::ExamRequest req;
    req.deserialize(payload);

    if (!sessionManager->is_session_valid(req.sessionToken)) {
        protocol::Message response(protocol::MsgCode::EXAM_FAILURE, "Invalid session");
        sendMessage(clientFd, response);
        return;
    }
    sessionManager->update_session(req.sessionToken);

    int examId;
    try {
        examId = std::stoi(req.examId);
    } catch (...) {
        protocol::Message response(protocol::MsgCode::EXAM_FAILURE, "Invalid exam ID");
        sendMessage(clientFd, response);
        return;
    }

    if (logger::serverLogger) {
        logger::serverLogger->info("[TeacherExamController] Teacher reviewing exam ID: " + std::to_string(examId));
    }

    // Teachers can view any exam - no "already taken" check
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

bool TeacherExamController::sendMessage(int clientFd, const protocol::Message& msg) {
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
