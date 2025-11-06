#ifndef SERVER_H
#define SERVER_H

#include "server/session.h"
#include "server/database.h"
#include "server/user_manager.h"
#include "server/client_handler.h"
#include "server/lesson_handler.h"
#include "server/exercise_handler.h"
#include "server/submission_handler.h"
#include "server/result_handler.h"
#include "server/exam_handler.h"
#include "server/exercise_loader.h"
#include "server/exam_loader.h"
#include <vector>
#include <set>
#include <memory>

namespace server {

class Server {
private:
    int serverSocket;
    int port;
    bool running;
    std::string dbConnInfo;
    
    std::shared_ptr<Database> database;
    std::shared_ptr<UserManager> userManager;
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ClientHandler> clientHandler;
    std::shared_ptr<LessonHandler> lessonHandler;
    std::shared_ptr<ExerciseHandler> exerciseHandler;
    std::shared_ptr<SubmissionHandler> submissionHandler;
    std::shared_ptr<ResultHandler> resultHandler;
    std::shared_ptr<ExamHandler> examHandler;

    std::shared_ptr<ExerciseLoader> exerciseLoader;
    std::shared_ptr<ExamLoader> examLoader;

    
    std::set<int> clientSockets;
    
    // Initialize server socket
    bool initSocket();
    
    // Accept new client connection
    int acceptClient();
    
    // Handle client data
    void handleClientData(int clientFd);
    
    // Remove client
    void removeClient(int clientFd);

public:
    Server(int port = 8080, const std::string& dbConn = "host=localhost port=5432 dbname=english_learning user=postgres password=yourpass");
    ~Server();

    // Start server
    bool start();
    
    // Main event loop with select()
    void run();
    
    // Stop server
    void stop();
};

} // namespace server

#endif // SERVER_H
