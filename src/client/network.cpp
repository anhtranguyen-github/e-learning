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
            userRole = utils::extractCookie(responseStr, "role");
            
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
            userRole.clear();
            
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
    userRole.clear();
    
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
    // Check if we already have a complete message in buffer
    uint32_t msgLen = protocol::Message::getFullLength(receiveBuffer);
    
    while (msgLen == 0) {
        // Need more data
        std::vector<uint8_t> newData = receiveData();
        if (newData.empty()) {
            // Check again, maybe receiveData timed out but we have partial data?
            // Actually receiveData throws if no data received after timeout?
            // The original receiveData returned empty vector on timeout.
            // But receiveMessage threw runtime_error if empty.
            // We should probably throw if we can't get a full message.
            throw std::runtime_error("Timeout waiting for message");
        }
        
        receiveBuffer.insert(receiveBuffer.end(), newData.begin(), newData.end());
        msgLen = protocol::Message::getFullLength(receiveBuffer);
    }
    
    // Extract message
    std::vector<uint8_t> msgData(receiveBuffer.begin(), receiveBuffer.begin() + msgLen);
    receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + msgLen);
    
    return protocol::Message::deserialize(msgData);
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

std::vector<protocol::Message> NetworkClient::pollMessages() {
    std::vector<protocol::Message> messages;
    
    if (sockfd < 0) return messages;

    // Try to read data without blocking
    std::vector<uint8_t> buffer(4096);
    ssize_t received = recv(sockfd, buffer.data(), buffer.size(), 0);
    
    if (received > 0) {
        buffer.resize(received);
        receiveBuffer.insert(receiveBuffer.end(), buffer.begin(), buffer.end());
    } else if (received == 0) {
        // Disconnected?
        connected = false;
        return messages;
    } else {
        // Error or EWOULDBLOCK
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            // Real error
            connected = false;
            return messages;
        }
    }
    
    // Process all complete messages in buffer
    while (true) {
        uint32_t msgLen = protocol::Message::getFullLength(receiveBuffer);
        
        if (msgLen == 0) {
            break;
        }
        
        std::vector<uint8_t> msgData(receiveBuffer.begin(), receiveBuffer.begin() + msgLen);
        receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + msgLen);
        
        try {
            messages.push_back(protocol::Message::deserialize(msgData));
        } catch (...) {
            // Should not happen if getFullLength returned > 0
        }
    }
    
    return messages;
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

bool NetworkClient::requestPendingSubmissions() {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot request pending submissions");
        }
        return false;
    }

    Payloads::PendingSubmissionsRequest req;
    req.sessionToken = sessionToken;
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::PENDING_SUBMISSIONS_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send pending submissions request");
        }
        return false;
    }

    return true;
}

bool NetworkClient::submitGrade(const std::string& resultId, const std::string& score, const std::string& feedback) {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot submit grade");
        }
        return false;
    }

    Payloads::GradeSubmissionRequest req;
    req.sessionToken = sessionToken;
    req.resultId = resultId;
    req.score = score;
    req.feedback = feedback;
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::GRADE_SUBMISSION_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send grade submission request");
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

    Payloads::ResultListRequest req;
    req.sessionToken = sessionToken;
    req.targetType = ""; // Fetch all
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::RESULT_LIST_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send result list request");
        }
        return false;
    }

    return true;
}

bool NetworkClient::requestResultDetail(const std::string& targetType, const std::string& targetId) {
    if (!connected || sessionToken.empty()) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Cannot request result detail: Not connected or not logged in");
        }
        return false;
    }

    Payloads::ResultDetailRequest req;
    req.sessionToken = sessionToken;
    req.targetType = targetType;
    req.targetId = targetId;
    
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::RESULT_DETAIL_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send result detail request");
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

bool NetworkClient::requestExam(int examId) {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot request exam");
        }
        return false;
    }

    Payloads::ExamRequest req;
    req.sessionToken = sessionToken;
    req.examId = std::to_string(examId);
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::EXAM_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send exam request");
        }
        return false;
    }

    return true;
}

bool NetworkClient::sendPrivateMessage(const std::string& recipient, const std::string& content, const std::string& type) {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot send private message");
        }
        return false;
    }

    Payloads::PrivateMessageRequest req;
    req.sessionToken = sessionToken;
    req.recipient = recipient;
    req.content = content;
    req.messageType = type;
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



bool NetworkClient::requestRecentChats() {
    if (!connected || !loggedIn) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Not logged in - cannot request recent chats");
        }
        return false;
    }

    Payloads::RecentChatsRequest req;
    req.sessionToken = sessionToken;
    std::string payload = req.serialize();
    protocol::Message msg(protocol::MsgCode::RECENT_CHATS_REQUEST, payload);

    if (!sendMessage(msg)) {
        if (logger::clientLogger) {
            logger::clientLogger->error("Failed to send recent chats request");
        }
        return false;
    }

    return true;
}

} // namespace client
