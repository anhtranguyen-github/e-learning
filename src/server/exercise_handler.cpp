#include "server/exercise_handler.h"
#include "common/logger.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <vector>

namespace server {

// ============================================================================
// Constructor
// ============================================================================

ExerciseHandler::ExerciseHandler(std::shared_ptr<SessionManager> sm, std::shared_ptr<ExerciseLoader> el)
    : sessionManager(sm), exerciseLoader(el) {
}

// ============================================================================
// Helper Functions
// ============================================================================

bool ExerciseHandler::sendMessage(int clientFd, const protocol::Message& msg) {
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

ExerciseType ExerciseHandler::parseExerciseType(const std::string& typeStr) {
    if (typeStr == "question") return ExerciseType::QUESTION;
    if (typeStr == "options") return ExerciseType::OPTIONS;
    if (typeStr == "answer") return ExerciseType::ANSWER;
    if (typeStr == "explanation") return ExerciseType::EXPLANATION;
    if (typeStr == "full") return ExerciseType::FULL;
    
    // Default to FULL if unknown
    return ExerciseType::FULL;
}

// ============================================================================
// Message Handlers
// ============================================================================

void ExerciseHandler::handleExerciseListRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Handling EXERCISE_LIST_REQUEST from fd=" + std::to_string(clientFd) + ", payload: " + payload);
    }
    
    // Parse payload: <session_token>[;<type>;<level>;<lesson_id>]
    std::vector<std::string> parts;
    std::istringstream iss(payload);
    std::string part;
    
    while (std::getline(iss, part, ';')) {
        parts.push_back(part);
    }
    
    if (parts.empty()) {
        std::string errorMsg = "Empty payload in EXERCISE_LIST_REQUEST from fd=" + std::to_string(clientFd);
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::EXERCISE_LIST_FAILURE, errorMsg);
        sendMessage(clientFd, response);
        return;
    }
    
    std::string sessionToken = parts[0];
    std::string type = parts.size() > 1 ? parts[1] : "";
    std::string level = parts.size() > 2 ? parts[2] : "";
    int lessonId = parts.size() > 3 ? std::stoi(parts[3]) : -1;
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Parsed request - token: " + sessionToken +
                                  ", type: " + (type.empty() ? "<none>" : type) +
                                  ", level: " + (level.empty() ? "<none>" : level) +
                                  ", lessonId: " + (lessonId == -1 ? "<none>" : std::to_string(lessonId)));
    }
    
    // Validate session token
    if (!sessionManager->is_session_valid(sessionToken)) {
        std::string errorMsg = "Invalid or expired session token in EXERCISE_LIST_REQUEST from fd=" +
                             std::to_string(clientFd);
        if (logger::serverLogger) {
            logger::serverLogger->warn("[WARN] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::EXERCISE_LIST_FAILURE, errorMsg);
        sendMessage(clientFd, response);
        return;
    }
    
    // Update session activity
    sessionManager->update_session(sessionToken);
    
    // Load exercises from database
    ExerciseList exerciseList;
    
    try {
        if (type.empty() && level.empty() && lessonId == -1) {
            if (logger::serverLogger) {
                logger::serverLogger->debug("[DEBUG] Loading all exercises");
            }
            exerciseList = exerciseLoader->loadAllExercises();
        } else {
            if (logger::serverLogger) {
                logger::serverLogger->debug("[DEBUG] Loading filtered exercises");
            }
            exerciseList = exerciseLoader->loadExercisesByFilter(type, level, lessonId);
        }
        
        int exerciseCount = exerciseList.count();
        if (logger::serverLogger) {
            logger::serverLogger->info("[INFO] Loaded " + std::to_string(exerciseCount) + " exercises");
        }
        
        // Serialize exercise list for network transmission
        std::string serializedList = exerciseList.serializeForNetwork();
        
        // Send success response
        protocol::Message response(protocol::MsgCode::EXERCISE_LIST_SUCCESS, serializedList);
        
        if (sendMessage(clientFd, response)) {
            if (logger::serverLogger) {
                logger::serverLogger->info("[INFO] Successfully sent " + std::to_string(exerciseCount) +
                                         " exercises to fd=" + std::to_string(clientFd));
            }
        } else {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Failed to send exercise list to fd=" +
                                          std::to_string(clientFd));
            }
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Exception in handleExerciseListRequest: " + std::string(e.what());
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] " + errorMsg);
        }
        protocol::Message response(protocol::MsgCode::EXERCISE_LIST_FAILURE, errorMsg);
        sendMessage(clientFd, response);
    }
}

void ExerciseHandler::handleStudyExerciseRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("Handling STUDY_EXERCISE_REQUEST from fd=" + std::to_string(clientFd));
    }
    
    // Parse payload: <session_token>;<exercise_id>;<exercise_type>
    std::vector<std::string> parts;
    std::istringstream iss(payload);
    std::string part;
    
    while (std::getline(iss, part, ';')) {
        parts.push_back(part);
    }
    
    if (parts.size() < 3) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid payload in STUDY_EXERCISE_REQUEST from fd=" +
                                       std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::STUDY_EXERCISE_FAILURE, "Invalid request format");
        sendMessage(clientFd, response);
        return;
    }
    
    std::string sessionToken = parts[0];
    std::string exerciseIdStr = parts[1];
    std::string exerciseTypeStr = parts[2];
    
    // Validate session token
    if (!sessionManager->is_session_valid(sessionToken)) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Invalid session token in STUDY_EXERCISE_REQUEST from fd=" +
                                      std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::STUDY_EXERCISE_FAILURE, "Invalid or expired session");
        sendMessage(clientFd, response);
        return;
    }
    
    // Update session activity
    sessionManager->update_session(sessionToken);
    
    // Parse exercise ID
    int exerciseId;
    try {
        exerciseId = std::stoi(exerciseIdStr);
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid exercise_id in STUDY_EXERCISE_REQUEST from fd=" +
                                       std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::STUDY_EXERCISE_FAILURE, "Invalid exercise ID");
        sendMessage(clientFd, response);
        return;
    }
    
    // Parse exercise type
    ExerciseType exerciseType = parseExerciseType(exerciseTypeStr);
    
    // Load exercise from database
    Exercise exercise = exerciseLoader->loadExerciseById(exerciseId);
    
    // Check if exercise was found
    if (exercise.getExerciseId() == -1) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Exercise " + std::to_string(exerciseId) + " not found for fd=" +
                                      std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::STUDY_EXERCISE_FAILURE, "Exercise not found");
        sendMessage(clientFd, response);
        return;
    }
    
    // Serialize only the requested content type
    std::string serializedContent = exercise.serializeForNetwork(exerciseType);
    
    // Send success response
    protocol::Message response(protocol::MsgCode::STUDY_EXERCISE_SUCCESS, serializedContent);
    
    if (sendMessage(clientFd, response)) {
        if (logger::serverLogger) {
            logger::serverLogger->info("Sent exercise " + std::to_string(exerciseId) +
                                      " (" + exerciseTypeStr + ") to fd=" + std::to_string(clientFd));
        }
    } else {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to send exercise content to fd=" + std::to_string(clientFd));
        }
    }
}

} // namespace server