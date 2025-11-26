#include "server/client_handler.h"
#include "server/session.h"
#include "server/user_manager.h"
#include "server/handler_registry.h"
#include "common/logger.h"
#include "common/utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

namespace server {

ClientHandler::ClientHandler(
    std::shared_ptr<SessionManager> sm,
    std::shared_ptr<UserManager> um,
    std::shared_ptr<HandlerRegistry> hr)
    : sessionManager_(sm), userManager_(um), handlerRegistry_(hr) {}

void ClientHandler::processMessage(int clientFd, const std::vector<uint8_t>& data) {
    clientFd_ = clientFd;
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
                handleLoginRequest(msg);
                break;
            case protocol::MsgCode::LOGOUT_REQUEST:
                handleLogoutRequest(msg);
                break;
            case protocol::MsgCode::HEARTBEAT:
                handleHeartbeat(msg);
                break;
            case protocol::MsgCode::DISCONNECT_REQUEST:
                handleDisconnectRequest();
                break;

            case protocol::MsgCode::LESSON_LIST_REQUEST:
                handlerRegistry_->get_lesson_handler()->handleLessonListRequest(clientFd, msg);
                break;
            case protocol::MsgCode::STUDY_LESSON_REQUEST:
                handlerRegistry_->get_lesson_handler()->handleStudyLessonRequest(clientFd, msg);
                break;
            case protocol::MsgCode::EXERCISE_LIST_REQUEST:
                handlerRegistry_->get_exercise_handler()->handleExerciseListRequest(clientFd, msg);
                break;
            case protocol::MsgCode::STUDY_EXERCISE_REQUEST:
                handlerRegistry_->get_exercise_handler()->handleStudyExerciseRequest(clientFd, msg);
                break;
            
            case protocol::MsgCode::SEND_CHAT_PRIVATE_REQUEST:
            case protocol::MsgCode::CHAT_HISTORY_REQUEST:
                handlerRegistry_->get_chat_handler()->handle_chat_message(this, msg);
                break;

            case protocol::MsgCode::MULTIPLE_CHOICE_REQUEST:
            case protocol::MsgCode::FILL_IN_REQUEST:
            case protocol::MsgCode::SENTENCE_ORDER_REQUEST:
            case protocol::MsgCode::REWRITE_SENTENCE_REQUEST:
            case protocol::MsgCode::WRITE_PARAGRAPH_REQUEST:
            case protocol::MsgCode::SPEAKING_TOPIC_REQUEST:
                handlerRegistry_->get_exercise_handler()->handleSpecificExerciseRequest(clientFd, msg);
                break;

            case protocol::MsgCode::SUBMIT_ANSWER_REQUEST:
                handlerRegistry_->get_submission_handler()->handleSubmission(clientFd, msg);
                break;

            case protocol::MsgCode::RESULT_LIST_REQUEST:
                handlerRegistry_->get_result_handler()->handleResultRequest(clientFd, msg);
                break;

            case protocol::MsgCode::EXAM_LIST_REQUEST:
                handlerRegistry_->get_exam_handler()->handleExamRequest(clientFd, msg);
                break;

            default:
                if (logger::serverLogger) {
                    logger::serverLogger->warn("Unknown message code received: " + std::to_string(static_cast<uint16_t>(msg.code)));
                }
                response = protocol::Message(protocol::MsgCode::UNKNOWN_COMMAND_FAILURE, "Unknown command");
                send_message(response);
                break;
        }
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Error processing message from fd=" + std::to_string(clientFd) + ": " + e.what());
        }
        response = protocol::Message(protocol::MsgCode::GENERAL_FAILURE, "Server error processing message");
        send_message(response);
    }
}
void ClientHandler::handleLoginRequest(const protocol::Message& msg) {
    std::string payload = msg.toString();
    std::string username, password;
    
    if (!utils::parseLoginCredentials(payload, username, password)) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid login credentials format from fd=" + std::to_string(clientFd_));
        }
        protocol::Message response(protocol::MsgCode::LOGIN_FAILURE, "Invalid credentials format");
        send_message(response);
        return;
    }

    // Verify credentials
    if (userManager_->verify_credentials(username, password)) {
        // Get user ID
        int userId = userManager_->get_user_id(username);
        
        // Create session
        std::string sessionId = sessionManager_->create_session(userId, clientFd_);
        
        // Register client with UserManager
        userManager_->add_client(userId, this);

                    protocol::Message response(protocol::MsgCode::LOGIN_SUCCESS, "session_id=" + sessionId);        send_message(response);
        
        if (logger::serverLogger) {
            logger::serverLogger->info("User " + username + " logged in successfully (fd=" + 
                                      std::to_string(clientFd_) + ")");
        }
    } else {
        if (logger::serverLogger) {
            logger::serverLogger->warn("Failed login attempt for user " + username + 
                                      " (fd=" + std::to_string(clientFd_) + ")");
        }
        protocol::Message response(protocol::MsgCode::LOGIN_FAILURE, "Invalid credentials");
        send_message(response);
    }
}

void ClientHandler::handleLogoutRequest(const protocol::Message& msg) {
    std::string sessionId = msg.toString();
    
    if (sessionId.empty()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("No session token in logout request from fd=" + 
                                       std::to_string(clientFd_));
        }
        protocol::Message response(protocol::MsgCode::GENERAL_FAILURE, "No session token");
        send_message(response);
        return;
    }

    // Validate session
    if (!sessionManager_->is_session_valid(sessionId)) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Invalid session token in logout request from fd=" + 
                                       std::to_string(clientFd_));
        }
        protocol::Message response(protocol::MsgCode::GENERAL_FAILURE, "Invalid session");
        send_message(response);
        return;
    }

    // Unregister client
    int userId = sessionManager_->get_user_id_by_session(sessionId);
    if (userId != -1) {
        userManager_->remove_client(userId);
    }

    // Remove session
    sessionManager_->remove_session(sessionId);
    
    // Send success response
    protocol::Message response(protocol::MsgCode::LOGOUT_SUCCESS, "Logout successful");
    send_message(response);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("User logged out successfully (fd=" + 
                                  std::to_string(clientFd_) + ")");
    }
}

void ClientHandler::handleHeartbeat(const protocol::Message& msg) {
    std::string sessionId = msg.toString();
    
    if (sessionId.empty()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("No session token in heartbeat from fd=" + 
                                       std::to_string(clientFd_));
        }
        return;
    }

    // Validate and update session
    if (sessionManager_->is_session_valid(sessionId)) {
        sessionManager_->update_session(sessionId);
        
        if (logger::heartbeatLogger) {
            logger::heartbeatLogger->info("Heartbeat from session " + sessionId);
        }
    } else {
        if (logger::heartbeatLogger) {
            logger::heartbeatLogger->warn("Invalid session in heartbeat from fd=" + 
                                      std::to_string(clientFd_));
        }
    }
}

void ClientHandler::handleDisconnectRequest() {
    // Send acknowledgment
    protocol::Message response(protocol::MsgCode::DISCONNECT_ACK, "Disconnect acknowledged");
    send_message(response);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Disconnect request from fd=" + std::to_string(clientFd_));
    }
}

void ClientHandler::handleClientDisconnect(int clientFd) {
    // Unregister client
    int userId = sessionManager_->get_user_id_by_fd(clientFd);
    if (userId != -1) {
        userManager_->remove_client(userId);
    }

    // Remove session if exists
    sessionManager_->remove_session_by_fd(clientFd);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Client disconnected (fd=" + std::to_string(clientFd) + ")");
    }
}

bool ClientHandler::send_message(const protocol::Message& msg) {
    std::vector<uint8_t> data = msg.serialize();
    
    ssize_t sent = send(clientFd_, data.data(), data.size(), 0);
    
    if (sent < 0) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Failed to send message to fd=" + std::to_string(clientFd_));
        }
        return false;
    }
    
    return true;
}

int ClientHandler::get_user_id() const {
    return sessionManager_->get_user_id_by_fd(clientFd_);
}

} // namespace server

