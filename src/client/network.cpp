#include "client/network.h"
#include "common/payloads.h"
#include "common/logger.h"
#include "common/utils.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>

namespace client {

NetworkClient::NetworkClient(const std::string& host, int port)
    : sockfd(-1), serverHost(host), serverPort(port), 
      connected(false), loggedIn(false), heartbeatInterval(10) {
}

NetworkClient::~NetworkClient() {
    disconnect();
}

bool NetworkClient::connect() {
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to create socket");
        }
        return false;
    }

    // Set non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    // Server address
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    
    if (inet_pton(AF_INET, serverHost.c_str(), &serverAddr.sin_addr) <= 0) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Invalid server address");
        }
        close(sockfd);
        sockfd = -1;
        return false;
    }

    // Connect (non-blocking)
    int result = ::connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result < 0 && errno != EINPROGRESS) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to connect to server");
        }
        close(sockfd);
        sockfd = -1;
        return false;
    }

    // Wait for connection to complete
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sockfd, &writefds);
    
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    if (select(sockfd + 1, nullptr, &writefds, nullptr, &timeout) <= 0) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Connection timeout");
        }
        close(sockfd);
        sockfd = -1;
        return false;
    }

    connected = true;
    
    if (logger::clientLogger) {
        logger::clientLogger->info("Connected to server " + serverHost + ":" + std::to_string(serverPort));
    }
    
    return true;
}

void NetworkClient::disconnect() {
    if (sockfd >= 0) {
        if (loggedIn) {
            try {
                logout();
            } catch (const std::exception& e) {
                if (logger::clientLogger) {
                    logger::clientLogger->error("Error during logout in disconnect: " + std::string(e.what()));
                }
            }
        }
        
        // Send disconnect request
        try {
            protocol::Message msg(protocol::MsgCode::DISCONNECT_REQUEST, "");
            sendMessage(msg);
        } catch (...) {
            // Ignore errors during disconnect
        }
        
        close(sockfd);
        sockfd = -1;
        connected = false;
        loggedIn = false;
        
        if (logger::clientLogger) {
            logger::clientLogger->info("Disconnected from server");
        }
    }
}

bool NetworkClient::login(const std::string& username, const std::string& password) {
    if (!connected) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not connected to server");
        }
        return false;
    }

    // Send login request
    Payloads::LoginRequest req;
    req.username = username;
    req.password = password;
    std::string payload = req.serialize();
    protocol::Message loginMsg(protocol::MsgCode::LOGIN_REQUEST, payload);
    
    if (!sendMessage(loginMsg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send login request");
        }
        return false;
    }

    // Wait for response
    try {
        protocol::Message response = receiveMessage();
        
        if (response.code == protocol::MsgCode::LOGIN_SUCCESS) {
            // Extract session token from response
            std::string responseStr = response.toString();
            sessionToken = utils::extractCookie(responseStr, "session_id");
            
            if (sessionToken.empty()) {
                if (logger::clientLogger) {
                    logger::clientLogger->error("Failed to extract session token");
                }
                return false;
            }
            
            loggedIn = true;
            lastHeartbeat = std::chrono::steady_clock::now();
            
            if (logger::clientLogger) {
                logger::clientLogger->info("Login successful for user: " + username);
            }
            
            return true;
        } else if (response.code == protocol::MsgCode::LOGIN_FAILURE) {
            if (logger::clientLogger) {
                logger::clientLogger->error("Login failed: Invalid credentials");
            }
            return false;
        }
    } catch (const std::exception& e) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Error receiving login response: " + std::string(e.what()));
        }
        return false;
    }
    
    if (logger::clientLogger) {
        logger::clientLogger->error("Unexpected response from server");
    }
    return false;
}

bool NetworkClient::logout() {
    if (!loggedIn) {
        return true;
    }

    // Send logout request with session token
    if (logger::clientLogger) {
        logger::clientLogger->debug("Sending logout request with token: " + sessionToken);
    }
    std::string payload = sessionToken;
    protocol::Message logoutMsg(protocol::MsgCode::LOGOUT_REQUEST, payload);
    
    if (!sendMessage(logoutMsg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send logout request");
        }
        return false;
    }

    // Wait for response
    try {
        protocol::Message response = receiveMessage();
        
        if (response.code == protocol::MsgCode::LOGOUT_SUCCESS) {
            loggedIn = false;
            sessionToken.clear();
            
            if (logger::clientLogger) {
                logger::clientLogger->info("Logout successful");
            }
            
            return true;
        }
    } catch (const std::exception& e) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Error receiving logout response: " + std::string(e.what()));
        }
    }
    
    // Even if logout failed (e.g. invalid session), we should clear local session state
    // because the server likely already considers us logged out.
    loggedIn = false;
    sessionToken.clear();
    
    return false;
}

bool NetworkClient::sendHeartbeat() {
    if (!loggedIn) {
        return false;
    }

    std::string payload = sessionToken;
    protocol::Message heartbeatMsg(protocol::MsgCode::HEARTBEAT, payload);
    
    if (sendMessage(heartbeatMsg)) {
        lastHeartbeat = std::chrono::steady_clock::now();
        
        if (logger::clientLogger) {
            logger::clientLogger->debug("Heartbeat sent");
        }
        
        return true;
    }
    
    return false;
}

bool NetworkClient::shouldSendHeartbeat() {
    if (!loggedIn) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastHeartbeat).count();
    
    return elapsed >= heartbeatInterval;
}

bool NetworkClient::sendMessage(const protocol::Message& msg) {
    if (sockfd < 0) {
        return false;
    }

    std::vector<uint8_t> data = msg.serialize();
    return sendData(data);
}

protocol::Message NetworkClient::receiveMessage() {
    std::vector<uint8_t> data = receiveData();
    
    if (data.empty()) {
        throw std::runtime_error("No data received");
    }
    
    return protocol::Message::deserialize(data);
}

bool NetworkClient::sendData(const std::vector<uint8_t>& data) {
    if (sockfd < 0 || data.empty()) {
        return false;
    }

    size_t totalSent = 0;
    while (totalSent < data.size()) {
        ssize_t sent = send(sockfd, data.data() + totalSent, data.size() - totalSent, 0);
        
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Wait for socket to be writable
                fd_set writefds;
                FD_ZERO(&writefds);
                FD_SET(sockfd, &writefds);
                
                struct timeval timeout;
                timeout.tv_sec = 5;
                timeout.tv_usec = 0;
                
                if (select(sockfd + 1, nullptr, &writefds, nullptr, &timeout) <= 0) {
                    return false;
                }
                continue;
            }
            return false;
        }
        
        totalSent += sent;
    }
    
    return true;
}

std::vector<uint8_t> NetworkClient::receiveData() {
    if (sockfd < 0) {
        return {};
    }

    std::vector<uint8_t> buffer(4096);
    
    // Wait for data to be available
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    
    if (select(sockfd + 1, &readfds, nullptr, nullptr, &timeout) <= 0) {
        return {};
    }

    ssize_t received = recv(sockfd, buffer.data(), buffer.size(), 0);
    
    if (received <= 0) {
        return {};
    }
    
    buffer.resize(received);
    return buffer;
}

bool NetworkClient::requestLessonList(const std::string& topic, const std::string& level) {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot request lesson list");
        }
        return false;
    }

    // Build payload using Payloads
    Payloads::LessonListRequest req;
    req.sessionToken = sessionToken;
    req.topic = topic;
    req.level = level;
    std::string payload = req.serialize();

    if (logger::clientLogger) {
        logger::clientLogger->debug("Sending LESSON_LIST_REQUEST with payload: " + payload);
    }

    // Send lesson list request
    protocol::Message msg(protocol::MsgCode::LESSON_LIST_REQUEST, payload);
    
    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send LESSON_LIST_REQUEST");
        }
        return false;
    }

    if (logger::clientLogger) {
        logger::clientLogger->debug("Successfully sent LESSON_LIST_REQUEST");
    }

    return true;
}

bool NetworkClient::requestStudyLesson(int lessonId, const std::string& lessonType) {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot request lesson");
        }
        return false;
    }

    // Build payload using Payloads
    Payloads::StudyLessonRequest req;
    req.sessionToken = sessionToken;
    req.lessonId = std::to_string(lessonId);
    req.lessonType = lessonType;
    std::string payload = req.serialize();

    // Send study lesson request
    protocol::Message msg(protocol::MsgCode::STUDY_LESSON_REQUEST, payload);
    
    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send STUDY_LESSON_REQUEST");
        }
        return false;
    }

    if (logger::clientLogger) {
        logger::clientLogger->debug("Sent STUDY_LESSON_REQUEST for lesson " + std::to_string(lessonId));
    }

    return true;
}

bool NetworkClient::requestExercise(protocol::MsgCode exerciseType, int exerciseId) {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot request exercise");
        }
        return false;
    }

    Payloads::SpecificExerciseRequest req;
    req.sessionToken = sessionToken;
    req.exerciseId = std::to_string(exerciseId);
    std::string payload = req.serialize();
    protocol::Message msg(exerciseType, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send exercise request");
        }
        return false;
    }

    return true;
}

bool NetworkClient::submitAnswer(const std::string& targetType, int targetId, const std::string& answer) {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot submit answer");
        }
        return false;
    }

    Payloads::SubmitAnswerRequest req;
    req.sessionToken = sessionToken;
    req.targetType = targetType;
    req.targetId = std::to_string(targetId);
    req.answer = answer;
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::SUBMIT_ANSWER_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send submit answer request");
        }
        return false;
    }

    return true;
}

bool NetworkClient::requestResultList() {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot request result list");
        }
        return false;
    }

    protocol::Message msg(protocol::MsgCode::RESULT_LIST_REQUEST, sessionToken);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send result list request");
        }
        return false;
    }

    return true;
}

bool NetworkClient::requestExercises() {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot request exercises");
        }
        return false;
    }

    Payloads::ExerciseListRequest req;
    req.sessionToken = sessionToken;
    // Default empty filters for now as per original code
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::EXERCISE_LIST_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send get exercises request");
        }
        return false;
    }

    return true;
}

bool NetworkClient::requestExams() {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot request exams");
        }
        return false;
    }

    Payloads::ExamListRequest req;
    req.sessionToken = sessionToken;
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::EXAM_LIST_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send get exams request");
        }
        return false;
    }

    return true;
}

bool NetworkClient::sendPrivateMessage(const std::string& recipient, const std::string& message) {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot send private message");
        }
        return false;
    }

    Payloads::PrivateMessageRequest req;
    req.sessionToken = sessionToken;
    req.recipient = recipient;
    req.message = message;
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::SEND_CHAT_PRIVATE_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send private message");
        }
        return false;
    }

    return true;
}

bool NetworkClient::requestChatHistory(const std::string& otherUser) {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot request chat history");
        }
        return false;
    }

    Payloads::ChatHistoryRequest req;
    req.sessionToken = sessionToken;
    req.otherUser = otherUser;
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::CHAT_HISTORY_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send chat history request");
        }
        return false;
    }

    return true;
}

} // namespace client
