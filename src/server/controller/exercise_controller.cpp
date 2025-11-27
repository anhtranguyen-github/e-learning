#include "server/controller/exercise_controller.h"
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

ExerciseController::ExerciseController(std::shared_ptr<SessionManager> sessionMgr, std::shared_ptr<ExerciseRepository> exerciseRepo)
    : sessionManager(sessionMgr), exerciseRepository(exerciseRepo) {
}

// ============================================================================
// Helper Functions
// ============================================================================

bool ExerciseController::sendMessage(int clientFd, const protocol::Message& msg) {
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

ExerciseType ExerciseController::parseExerciseType(const std::string& typeStr) {
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

void ExerciseController::handleExerciseListRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Handling EXERCISE_LIST_REQUEST from fd=" + std::to_string(clientFd) + ", payload: " + payload);
    }
    
    Payloads::ExerciseListRequest req;
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
            exerciseList = exerciseRepository->loadAllExercises();
        } else {
            if (logger::serverLogger) {
                logger::serverLogger->debug("[DEBUG] Loading filtered exercises");
            }
            exerciseList = exerciseRepository->loadExercisesByFilter(lessonId, type, level);
        }
        
        int exerciseCount = exerciseList.count();
        if (logger::serverLogger) {
            logger::serverLogger->info("[INFO] Loaded " + std::to_string(exerciseCount) + " exercises");
        }
        
        // Serialize exercise list for network transmission
        // Serialize exercise list using DTOs
        std::vector<std::string> serializedDtos;
        for (const auto& exercise : exerciseList.getExercises()) {
            serializedDtos.push_back(exercise.toMetadataDTO().serialize());
        }
        std::string serializedList = std::to_string(serializedDtos.size());
        if (!serializedDtos.empty()) {
            serializedList += ";" + utils::join(serializedDtos, ';');
        }
        
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

void ExerciseController::handleStudyExerciseRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("Handling STUDY_EXERCISE_REQUEST from fd=" + std::to_string(clientFd));
    }
    
    Payloads::StudyExerciseRequest req;
    req.deserialize(payload);
    
    std::string sessionToken = req.sessionToken;
    std::string exerciseIdStr = req.exerciseId;
    std::string exerciseTypeStr = req.exerciseType;
    
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
    // ExerciseType exerciseType = parseExerciseType(exerciseTypeStr); // Unused with DTOs
    
    // Load exercise from database
    Exercise exercise = exerciseRepository->loadExerciseById(exerciseId);
    
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
    
    if (logger::serverLogger) {
        logger::serverLogger->info("ExerciseController: Found exercise " + std::to_string(exerciseId) + ", serializing...");
    }
    
    // Serialize only the requested content type
    // Serialize using DTO
    Payloads::ExerciseDTO dto = exercise.toDTO();
    std::string serializedContent = dto.serialize();
    
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

void ExerciseController::handleSpecificExerciseRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    Payloads::SpecificExerciseRequest req;
    req.deserialize(payload);
    
    std::string sessionToken = req.sessionToken;
    std::string exerciseIdStr = req.exerciseId;
    
    protocol::MsgCode successCode = static_cast<protocol::MsgCode>(static_cast<int>(msg.code) + 1);
    protocol::MsgCode failureCode = static_cast<protocol::MsgCode>(static_cast<int>(msg.code) + 2);
    
    if (!sessionManager->is_session_valid(sessionToken)) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("ExerciseController: Invalid session token for specific exercise request from fd=" + std::to_string(clientFd));
        }
        protocol::Message response(failureCode, "Invalid session");
        sendMessage(clientFd, response);
        return;
    }
    
    sessionManager->update_session(sessionToken);
    
    int exerciseId;
    try {
        exerciseId = std::stoi(exerciseIdStr);
    } catch (...) {
        if (logger::serverLogger) {
            logger::serverLogger->error("ExerciseController: Invalid exercise ID '" + exerciseIdStr + "' in specific exercise request from fd=" + std::to_string(clientFd));
        }
        protocol::Message response(failureCode, "Invalid exercise ID");
        sendMessage(clientFd, response);
        return;
    }
    
    if (logger::serverLogger) {
        logger::serverLogger->info("ExerciseController: Handling specific exercise request for ID: " + std::to_string(exerciseId));
    }
    
    Exercise exercise = exerciseRepository->loadExerciseById(exerciseId);
    
    if (exercise.getExerciseId() == -1) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("ExerciseController: Exercise " + std::to_string(exerciseId) + " not found for specific request from fd=" + std::to_string(clientFd));
        }
        protocol::Message response(failureCode, "Exercise not found");
        sendMessage(clientFd, response);
        return;
    }
    
    // For specific exercises, we usually return the question content
    // We can use serializeForNetwork with QUESTION type or FULL depending on needs.
    // The client expects the question text usually.
    // Use DTO
    Payloads::ExerciseDTO dto = exercise.toDTO();
    std::string content = dto.serialize();
    
    protocol::Message response(successCode, content);
    sendMessage(clientFd, response);
}

} // namespace server