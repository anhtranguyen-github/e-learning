#include "server/lesson_handler.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <vector>

namespace server {

// ============================================================================
// Constructor
// ============================================================================

LessonHandler::LessonHandler(std::shared_ptr<SessionManager> sm, std::shared_ptr<LessonLoader> ll)
    : sessionManager(sm), lessonLoader(ll) {
}

// ============================================================================
// Helper Functions
// ============================================================================

bool LessonHandler::sendMessage(int clientFd, const protocol::Message& msg) {
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

LessonType LessonHandler::parseLessonType(const std::string& typeStr) {
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

void LessonHandler::handleLessonListRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Handling LESSON_LIST_REQUEST from fd=" + std::to_string(clientFd) + 
                                  ", payload: " + payload);
    }
    
    // Parse payload: <session_token>[;<topic>;<level>]
    std::vector<std::string> parts;
    std::istringstream iss(payload);
    std::string part;
    
    while (std::getline(iss, part, ';')) {
        parts.push_back(part);
    }
    
    if (parts.empty()) {
        std::string errorMsg = "Empty payload in LESSON_LIST_REQUEST from fd=" + std::to_string(clientFd);
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::LESSON_LIST_FAILURE, errorMsg);
        sendMessage(clientFd, response);
        return;
    }
    
    std::string sessionToken = parts[0];
    std::string topic = parts.size() > 1 ? parts[1] : "";
    std::string level = parts.size() > 2 ? parts[2] : "";
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Parsed request - token: " + sessionToken + 
                                  ", topic: " + (topic.empty() ? "<none>" : topic) + 
                                  ", level: " + (level.empty() ? "<none>" : level));
    }
    
    // Validate session token
    if (!sessionManager->validateSession(sessionToken)) {
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
    sessionManager->updateLastActive(sessionToken);
    
    // Load lessons from database
    LessonList lessonList;
    
    try {
        if (topic.empty() && level.empty()) {
            if (logger::serverLogger) {
                logger::serverLogger->debug("[DEBUG] Loading all lessons");
            }
            lessonList = lessonLoader->loadAllLessons();
        } else {
            if (logger::serverLogger) {
                logger::serverLogger->debug("[DEBUG] Loading filtered lessons");
            }
            lessonList = lessonLoader->loadLessonsByFilter(topic, level);
        }
        
        int lessonCount = lessonList.count();
        if (logger::serverLogger) {
            logger::serverLogger->info("[INFO] Loaded " + std::to_string(lessonCount) + " lessons");
        }
        
        // Serialize lesson list for network transmission
        std::string serializedList = lessonList.serializeForNetwork();
        
        // Send success response
        protocol::Message response(protocol::MsgCode::LESSON_LIST_SUCCESS, serializedList);
        
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

void LessonHandler::handleStudyLessonRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("Handling STUDY_LESSON_REQUEST from fd=" + std::to_string(clientFd));
    }
    
    // Parse payload: <session_token>;<lesson_id>;<lesson_type>
    std::vector<std::string> parts;
    std::istringstream iss(payload);
    std::string part;
    
    while (std::getline(iss, part, ';')) {
        parts.push_back(part);
    }
    
    if (parts.size() < 3) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid payload in STUDY_LESSON_REQUEST from fd=" + 
                                       std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::STUDY_LESSON_FAILURE, "Invalid request format");
        sendMessage(clientFd, response);
        return;
    }
    
    std::string sessionToken = parts[0];
    std::string lessonIdStr = parts[1];
    std::string lessonTypeStr = parts[2];
    
    // Validate session token
    if (!sessionManager->validateSession(sessionToken)) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Invalid session token in STUDY_LESSON_REQUEST from fd=" + 
                                      std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::STUDY_LESSON_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }
    
    // Update session activity
    sessionManager->updateLastActive(sessionToken);
    
    // Parse lesson ID
    int lessonId;
    try {
        lessonId = std::stoi(lessonIdStr);
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid lesson_id in STUDY_LESSON_REQUEST from fd=" + 
                                       std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::STUDY_LESSON_FAILURE, "Invalid lesson ID");
        sendMessage(clientFd, response);
        return;
    }
    
    // Parse lesson type
    LessonType lessonType = parseLessonType(lessonTypeStr);
    
    // Load lesson from database
    Lesson lesson = lessonLoader->loadLessonById(lessonId);
    
    // Check if lesson was found
    if (lesson.getLessonId() == -1) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Lesson " + std::to_string(lessonId) + " not found for fd=" + 
                                      std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::STUDY_LESSON_FAILURE, "Lesson not found");
        sendMessage(clientFd, response);
        return;
    }
    
    // Serialize only the requested content type
    std::string serializedContent = lesson.serializeForNetwork(lessonType);
    
    // Send success response
    protocol::Message response(protocol::MsgCode::STUDY_LESSON_SUCCESS, serializedContent);
    
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
