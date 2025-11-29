#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

#include "common/protocol.h"
#include <string>
#include <vector>
#include <chrono>

namespace client {

class NetworkClient {
private:
    int sockfd;
    std::string serverHost;
    int serverPort;
    std::string sessionToken;
    bool connected;
    bool loggedIn;
    
    std::chrono::steady_clock::time_point lastHeartbeat;
    int heartbeatInterval; // seconds

    // Send raw data
    bool sendData(const std::vector<uint8_t>& data);
    
    // Receive raw data
    // Receive raw data
    std::vector<uint8_t> receiveData();
    
    // Buffer for incoming data
    std::vector<uint8_t> receiveBuffer;

public:
    NetworkClient(const std::string& host = "127.0.0.1", int port = 8080);
    ~NetworkClient();

    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const { return connected; }
    bool isLoggedIn() const { return loggedIn; }

    // Authentication
    bool login(const std::string& username, const std::string& password);
    bool logout();

    // Heartbeat
    bool sendHeartbeat();
    bool shouldSendHeartbeat();

    // Message sending/receiving
    bool sendMessage(const protocol::Message& msg);
    protocol::Message receiveMessage();
    
    // Non-blocking poll for messages
    std::vector<protocol::Message> pollMessages();

    // Session management
    std::string getSessionToken() const { return sessionToken; }
    void setSessionToken(const std::string& token) { sessionToken = token; }

    // Lesson management
    bool requestLessonList(const std::string& topic = "", const std::string& level = "");
    bool requestStudyLesson(int lessonId, const std::string& lessonType);

    // Exercise and result management
    bool requestExercise(protocol::MsgCode exerciseType, int exerciseId);
    bool submitAnswer(const std::string& targetType, int targetId, const std::string& answer);
    bool requestResultList();
    bool requestResultDetail(const std::string& targetType, const std::string& targetId);
    bool requestPendingSubmissions();
    bool submitGrade(const std::string& resultId, const std::string& score, const std::string& feedback);

    // Exercise and exam requests
    bool requestExercises();
    bool requestExams();
    bool requestExam(int examId);

    // Chat
    bool sendPrivateMessage(const std::string& recipient, const std::string& content, const std::string& type = "TEXT");
    bool requestChatHistory(const std::string& otherUser);
    bool requestRecentChats();

    // Socket file descriptor
    int getSocketFd() const { return sockfd; }
};

} // namespace client

#endif // CLIENT_NETWORK_H
