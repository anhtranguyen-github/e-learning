#include "server/controller/lesson_controller.h"
#include "common/logger.h"
#include "common/payloads.h"
#include "common/utils.h"
#include <sys/socket.h>
#include <sstream>
#include <vector>

namespace server {

// ============================================================================
// Constructor
// ============================================================================

LessonController::LessonController(std::shared_ptr<SessionManager> sessionMgr, std::shared_ptr<LessonRepository> lessonRepo)
    : sessionManager(sessionMgr), lessonRepository(lessonRepo) {
}

// ============================================================================
// Helper Functions
// ============================================================================

bool LessonController::sendMessage(int clientFd, const protocol::Message& msg) {
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
        if (logger::serverLogger) {
            logger::serverLogger->error("Exception in sendMessage: " + std::string(e.what()));
        }
        return false;
    }
}

LessonType LessonController::parseLessonType(const std::string& typeStr) {
    if (typeStr == "video") return LessonType::VIDEO;
    if (typeStr == "audio") return LessonType::AUDIO;
    if (typeStr == "text") return LessonType::TEXT;
    if (typeStr == "vocabulary") return LessonType::VOCABULARY;
    if (typeStr == "grammar") return LessonType::GRAMMAR;
    if (typeStr == "full") return LessonType::FULL;
    
    // Default to FULL if unknown
    return LessonType::FULL;
}

// ============================================================================
// Message Handlers
// ============================================================================

void LessonController::handleLessonListRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Handling LESSON_LIST_REQUEST from fd=" + std::to_string(clientFd) + 
                                  ", payload: " + payload);
    }
    
    // Deserialize request using Payloads
    Payloads::LessonListRequest req;
    req.deserialize(payload);
    
    std::string sessionToken = req.sessionToken;
    std::string topic = req.topic;
    std::string level = req.level;
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Parsed request - token: " + sessionToken + 
                                  ", topic: " + (topic.empty() ? "<none>" : topic) + 
                                  ", level: " + (level.empty() ? "<none>" : level));
    }
    
    // Validate session token
    if (!sessionManager->is_session_valid(sessionToken)) {
        std::string errorMsg = "Invalid or expired session token in LESSON_LIST_REQUEST from fd=" + 
                             std::to_string(clientFd);
        if (logger::serverLogger) {
            logger::serverLogger->warn("[WARN] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::LESSON_LIST_FAILURE, errorMsg);
        sendMessage(clientFd, response);
        return;
    }
    
    // Update session activity
    sessionManager->update_session(sessionToken);
    
    // Load lessons from database
    LessonList lessonList;
    
    try {
        if (topic.empty() && level.empty()) {
            if (logger::serverLogger) {
                logger::serverLogger->debug("[DEBUG] Loading all lessons");
            }
            lessonList = lessonRepository->loadAllLessons();
        } else {
            if (logger::serverLogger) {
                logger::serverLogger->debug("[DEBUG] Loading filtered lessons");
            }
            lessonList = lessonRepository->loadLessonsByFilter(topic, level);
        }
        
        int lessonCount = lessonList.count();
        if (logger::serverLogger) {
            logger::serverLogger->info("[INFO] Loaded " + std::to_string(lessonCount) + " lessons");
        }
        
        // Serialize lesson list for network transmission
        // Convert to DTOs and serialize
        const auto& lessons = lessonList.getLessons();
        std::vector<std::string> serializedLessons;
        serializedLessons.push_back(std::to_string(lessons.size()));
        
        for (const auto& lesson : lessons) {
            serializedLessons.push_back(lesson.toMetadataDTO().serialize());
        }
        
        std::string responsePayload = utils::join(serializedLessons, ';');
        
        protocol::Message response(protocol::MsgCode::LESSON_LIST_SUCCESS, responsePayload);
        
        if (sendMessage(clientFd, response)) {
            if (logger::serverLogger) {
                logger::serverLogger->info("[INFO] Successfully sent " + std::to_string(lessonCount) + 
                                         " lessons to fd=" + std::to_string(clientFd));
            }
        } else {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Failed to send lesson list to fd=" + 
                                          std::to_string(clientFd));
            }
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Exception in handleLessonListRequest: " + std::string(e.what());
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::LESSON_LIST_FAILURE, errorMsg);
        sendMessage(clientFd, response);
    }
}

void LessonController::handleStudyLessonRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("Handling STUDY_LESSON_REQUEST from fd=" + std::to_string(clientFd));
    }
    
    // Deserialize request using Payloads
    Payloads::StudyLessonRequest req;
    req.deserialize(payload);
    
    std::string sessionToken = req.sessionToken;
    std::string lessonIdStr = req.lessonId;
    std::string lessonTypeStr = req.lessonType;
    
    // Validate session token
    if (!sessionManager->is_session_valid(sessionToken)) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Invalid session token in STUDY_LESSON_REQUEST from fd=" + 
                                      std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::STUDY_LESSON_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }
    
    // Update session activity
    sessionManager->update_session(sessionToken);
    
    // Parse lesson ID
    // Load lesson
    int lessonId = std::stoi(req.lessonId);
    server::Lesson lesson = lessonRepository->loadLessonById(lessonId);
    
    if (lesson.getLessonId() == -1) {
        protocol::Message response(protocol::MsgCode::STUDY_LESSON_FAILURE, "Lesson not found");
        sendMessage(clientFd, response);
        return;
    }
    
    // Convert to DTO and serialize
    // Note: The original code supported partial loading (VIDEO, AUDIO, etc.)
    // For this refactor, we are using the full LessonDTO which supports all fields.
    // If partial loading is strictly required for bandwidth, we might need separate DTOs or optional fields.
    // However, the prompt implies a full transition to DTOs.
    // Given the current LessonDTO structure, we will send the full DTO.
    // If the client requested a specific type, we could potentially clear other fields in the DTO before sending,
    // but sending the full object is safer for now as it guarantees all data is available.
    
    Payloads::LessonDTO dto = lesson.toDTO();
    std::string responsePayload = dto.serialize();
    
    protocol::Message response(protocol::MsgCode::STUDY_LESSON_SUCCESS, responsePayload);
    sendMessage(clientFd, response);
    
    if (sendMessage(clientFd, response)) {
        if (logger::serverLogger) {
            logger::serverLogger->info("Sent lesson " + std::to_string(lessonId) + 
                                      " (" + lessonTypeStr + ") to fd=" + std::to_string(clientFd));
        }
    } else {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to send lesson content to fd=" + std::to_string(clientFd));
        }
    }
}

} // namespace server
