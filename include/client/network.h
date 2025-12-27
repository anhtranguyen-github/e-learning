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
    std::string userRole;
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
    bool registerUser(const std::string& username, const std::string& password);
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
    
    // User Role
    std::string getUserRole() const { return userRole; }

    // Lesson management
    bool requestLessonList(const std::string& topic = "", const std::string& level = "");
    bool requestStudyLesson(int lessonId, const std::string& lessonType);

    // Exercise and result management
    bool requestExercise(protocol::MsgCode exerciseType, int exerciseId);
    bool submitAnswer(const std::string& targetType, int targetId, const std::string& answer);
    bool requestResultList();
    bool requestResultDetail(const std::string& targetType, const std::string& targetId);
    bool requestPendingSubmissions();
    bool submitGrade(const std::string& resultId, const std::string& userId, const std::string& score, const std::string& feedback);
    // Exercise and exam requests
    bool requestExercises();
    bool requestExams();
    bool requestExam(int examId);            // Student exam request
    bool requestExamReview(int examId);      // Teacher exam review (no restrictions)

    // Chat
    bool sendPrivateMessage(const std::string& recipient, const std::string& content, const std::string& type = "TEXT");
    bool requestChatHistory(const std::string& otherUser);
    bool requestRecentChats();

    // Voice Calls
    bool initiateCall(const std::string& targetUser);
    bool answerCall(const std::string& callerUser);
    bool declineCall(const std::string& callerUser);
    bool endCall(const std::string& otherUser);

    // Admin Game Management
    bool requestCreateGame(const std::string& type, const std::string& level, const std::string& questionJson);
    bool requestUpdateGame(const std::string& gameId, const std::string& type, const std::string& level, const std::string& questionJson);
    bool requestDeleteGame(const std::string& gameId);

    // Game Play
    bool requestGameList();
    bool requestGameLevelList(const std::string& gameType);
    bool requestGameData(const std::string& gameId);
    bool submitGameResult(const std::string& gameId, const std::string& score, const std::string& detailsJson);

    // Socket file descriptor
    int getSocketFd() const { return sockfd; }
};

} // namespace client

#endif // CLIENT_NETWORK_H
