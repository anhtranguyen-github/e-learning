#include "server/client_handler.h"
#include "common/logger.h"
#include "common/utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include "server/lesson_handler.h"  

namespace server {

ClientHandler::ClientHandler(std::shared_ptr<SessionManager> sm, 
                           std::shared_ptr<UserManager> um,
                           std::shared_ptr<LessonHandler> lh,
                           std::shared_ptr<ExerciseHandler> eh,
                           std::shared_ptr<SubmissionHandler> sh,
                           std::shared_ptr<ExamHandler> exh,
                           std::shared_ptr<ResultHandler> rh)
    : sessionManager(sm), userManager(um), lessonHandler(lh), exerciseHandler(eh), submissionHandler(sh), examHandler(exh), resultHandler(rh) {
}

void ClientHandler::processMessage(int clientFd, const std::vector<uint8_t>& data) {
    protocol::Message response; // Declare response here
    
    if (data.empty()) {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Received empty data from fd=" + std::to_string(clientFd));
        }
        return;
    }

    try {
        protocol::Message msg = protocol::Message::deserialize(data);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("Received message code: " + std::to_string(static_cast<uint16_t>(msg.code)) +
                                      " from fd=" + std::to_string(clientFd));
        }

        switch (msg.code) {
            case protocol::MsgCode::LOGIN_REQUEST:
                handleLoginRequest(clientFd, msg);
                break;
            case protocol::MsgCode::LOGOUT_REQUEST:
                handleLogoutRequest(clientFd, msg);
                break;
            case protocol::MsgCode::HEARTBEAT:
                handleHeartbeat(clientFd, msg);
                break;
            case protocol::MsgCode::DISCONNECT_REQUEST:
                handleDisconnectRequest(clientFd);
                break;

            case protocol::MsgCode::LESSON_LIST_REQUEST:
                lessonHandler->handleLessonListRequest(clientFd, msg);
                break;
            case protocol::MsgCode::STUDY_LESSON_REQUEST:
                lessonHandler->handleStudyLessonRequest(clientFd, msg);
                break;
            case protocol::MsgCode::EXERCISE_LIST_REQUEST:
                exerciseHandler->handleExerciseListRequest(clientFd, msg);
                break;
            case protocol::MsgCode::STUDY_EXERCISE_REQUEST:
                exerciseHandler->handleStudyExerciseRequest(clientFd, msg);
                break;

            case protocol::MsgCode::MULTIPLE_CHOICE_REQUEST:
            case protocol::MsgCode::FILL_IN_REQUEST:
            case protocol::MsgCode::SENTENCE_ORDER_REQUEST:
            case protocol::MsgCode::REWRITE_SENTENCE_REQUEST:
            case protocol::MsgCode::WRITE_PARAGRAPH_REQUEST:
            case protocol::MsgCode::SPEAKING_TOPIC_REQUEST:
                // Generic handling for specific exercise types
                if (logger::serverLogger) {
                    logger::serverLogger->warn("Unhandled specific exercise type request: " + std::to_string(static_cast<uint16_t>(msg.code)));
                }
                // For now, we'll just log and send a failure
                response = protocol::Message(protocol::MsgCode::GENERAL_FAILURE, "Specific exercise type not yet implemented");
                sendMessage(clientFd, response);
                break;

            case protocol::MsgCode::SUBMIT_ANSWER_REQUEST:
                submissionHandler->handleSubmission(clientFd, msg);
                break;

            case protocol::MsgCode::RESULT_LIST_REQUEST:
                resultHandler->handleResultRequest(clientFd, msg);
                break;

            case protocol::MsgCode::EXAM_LIST_REQUEST:
                examHandler->handleExamRequest(clientFd, msg);
                break;

            // Responses from server to client (should not be received by server in this context)
            case protocol::MsgCode::LOGIN_SUCCESS:
            case protocol::MsgCode::LOGIN_FAILURE:
            case protocol::MsgCode::LOGOUT_SUCCESS:
            case protocol::MsgCode::LESSON_LIST_SUCCESS:
            case protocol::MsgCode::LESSON_LIST_FAILURE:
            case protocol::MsgCode::STUDY_LESSON_SUCCESS:
            case protocol::MsgCode::STUDY_LESSON_FAILURE:
            case protocol::MsgCode::EXERCISE_LIST_SUCCESS:
            case protocol::MsgCode::EXERCISE_LIST_FAILURE:
            case protocol::MsgCode::STUDY_EXERCISE_SUCCESS:
            case protocol::MsgCode::STUDY_EXERCISE_FAILURE:
            case protocol::MsgCode::MULTIPLE_CHOICE_SUCCESS:
            case protocol::MsgCode::MULTIPLE_CHOICE_FAILURE:
            case protocol::MsgCode::FILL_IN_SUCCESS:
            case protocol::MsgCode::FILL_IN_FAILURE:
            case protocol::MsgCode::SENTENCE_ORDER_SUCCESS:
            case protocol::MsgCode::SENTENCE_ORDER_FAILURE:
            case protocol::MsgCode::REWRITE_SENTENCE_SUCCESS:
            case protocol::MsgCode::REWRITE_SENTENCE_FAILURE:
            case protocol::MsgCode::WRITE_PARAGRAPH_SUCCESS:
            case protocol::MsgCode::WRITE_PARAGRAPH_FAILURE:
            case protocol::MsgCode::SPEAKING_TOPIC_SUCCESS:
            case protocol::MsgCode::SPEAKING_TOPIC_FAILURE:
            case protocol::MsgCode::SUBMIT_ANSWER_SUCCESS:
            case protocol::MsgCode::SUBMIT_ANSWER_FAILURE:
            case protocol::MsgCode::RESULT_LIST_SUCCESS:
            case protocol::MsgCode::RESULT_LIST_FAILURE:
            case protocol::MsgCode::EXAM_LIST_SUCCESS:
            case protocol::MsgCode::EXAM_LIST_FAILURE:
            case protocol::MsgCode::NOTIFICATION_PUSH:
            case protocol::MsgCode::DISCONNECT_ACK:
            case protocol::MsgCode::GENERAL_FAILURE:
            case protocol::MsgCode::UNKNOWN_COMMAND_FAILURE:
                if (logger::serverLogger) {
                    logger::serverLogger->warn("Received unexpected server response code: " + std::to_string(static_cast<uint16_t>(msg.code)) +
                                              " from fd=" + std::to_string(clientFd));
                }
                response = protocol::Message(protocol::MsgCode::GENERAL_FAILURE, "Received unexpected server response");
                sendMessage(clientFd, response);
                break;

            default:
                if (logger::serverLogger) {
                    logger::serverLogger->warn("Unknown message code received: " + std::to_string(static_cast<uint16_t>(msg.code)));
                }
                response = protocol::Message(protocol::MsgCode::UNKNOWN_COMMAND_FAILURE, "Unknown command");
                sendMessage(clientFd, response);
                break;
        }
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Error processing message from fd=" + std::to_string(clientFd) + ": " + e.what());
        }
        response = protocol::Message(protocol::MsgCode::GENERAL_FAILURE, "Server error processing message");
        sendMessage(clientFd, response);
    }
}
void ClientHandler::handleLoginRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    std::string username, password;
    
    if (!utils::parseLoginCredentials(payload, username, password)) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid login credentials format from fd=" + std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::LOGIN_FAILURE, "Invalid credentials format");
        sendMessage(clientFd, response);
        return;
    }

    // Verify credentials
    if (userManager->verifyCredentials(username, password)) {
        // Get user ID
        int userId = userManager->getUserId(username);
        
        // Create session
        std::string sessionId = sessionManager->createSession(username, userId, clientFd);
        
        // Send success response with cookie
        std::string cookieHeader = utils::createCookieHeader(sessionId);
        protocol::Message response(protocol::MsgCode::LOGIN_SUCCESS, cookieHeader);
        sendMessage(clientFd, response);
        
        if (logger::serverLogger) {
            logger::serverLogger->info("User " + username + " logged in successfully (fd=" + 
                                      std::to_string(clientFd) + ")");
        }
    } else {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Failed login attempt for user " + username + 
                                      " (fd=" + std::to_string(clientFd) + ")");
        }
        protocol::Message response(protocol::MsgCode::LOGIN_FAILURE, "Invalid credentials");
        sendMessage(clientFd, response);
    }
}

void ClientHandler::handleLogoutRequest(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    std::string sessionId = utils::extractCookie(payload, "session_id");
    
    if (sessionId.empty()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("No session token in logout request from fd=" + 
                                       std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::GENERAL_FAILURE, "No session token");
        sendMessage(clientFd, response);
        return;
    }

    // Validate session
    if (!sessionManager->validateSession(sessionId)) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid session token in logout request from fd=" + 
                                       std::to_string(clientFd));
        }
        protocol::Message response(protocol::MsgCode::GENERAL_FAILURE, "Invalid session");
        sendMessage(clientFd, response);
        return;
    }

    // Remove session
    sessionManager->removeSession(sessionId);
    
    // Send success response
    protocol::Message response(protocol::MsgCode::LOGOUT_SUCCESS, "Logout successful");
    sendMessage(clientFd, response);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("User logged out successfully (fd=" + 
                                  std::to_string(clientFd) + ")");
    }
}

void ClientHandler::handleHeartbeat(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    std::string sessionId = utils::extractCookie(payload, "session_id");
    
    if (sessionId.empty()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("No session token in heartbeat from fd=" + 
                                       std::to_string(clientFd));
        }
        return;
    }

    // Validate and update session
    if (sessionManager->validateSession(sessionId)) {
        sessionManager->updateLastActive(sessionId);
        
        if (logger::heartbeatLogger) {
            logger::heartbeatLogger->info("Heartbeat from session " + sessionId);
        }
    } else {
        if (logger::heartbeatLogger) {
            logger::heartbeatLogger->warn("Invalid session in heartbeat from fd=" + 
                                      std::to_string(clientFd));
        }
    }
}

void ClientHandler::handleDisconnectRequest(int clientFd) {
    // Send acknowledgment
    protocol::Message response(protocol::MsgCode::DISCONNECT_ACK, "Disconnect acknowledged");
    sendMessage(clientFd, response);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Disconnect request from fd=" + std::to_string(clientFd));
    }
}

void ClientHandler::handleClientDisconnect(int clientFd) {
    // Remove session if exists
    sessionManager->removeSessionByFd(clientFd);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Client disconnected (fd=" + std::to_string(clientFd) + ")");
    }
}

bool ClientHandler::sendMessage(int clientFd, const protocol::Message& msg) {
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

protocol::Message ClientHandler::receiveMessage(int clientFd) {
    std::vector<uint8_t> buffer(4096);
    
    ssize_t received = recv(clientFd, buffer.data(), buffer.size(), 0);
    
    if (received <= 0) {
        throw std::runtime_error("Failed to receive message");
    }
    
    buffer.resize(received);
    return protocol::Message::deserialize(buffer);
}

} // namespace server
