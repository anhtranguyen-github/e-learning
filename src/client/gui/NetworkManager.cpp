#include "NetworkManager.h"
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent), m_client(std::make_unique<client::NetworkClient>()) {
    
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(100); // Poll every 100ms
    connect(m_pollTimer, &QTimer::timeout, this, &NetworkManager::checkMessages);
}

NetworkManager::~NetworkManager() {
    if (m_client) {
        m_client->disconnect();
    }
}

void NetworkManager::connectToServer(const QString &host, int port) {
    // Re-create client with new host and port
    m_client = std::make_unique<client::NetworkClient>(host.toStdString(), port);
    
    bool success = m_client->connect();
    emit connectionStatusChanged(success);
    
    if (success) {
        // Start polling for messages only after login usually, but we might receive initial handshake?
        // The current protocol doesn't send anything immediately after connect, only after login.
        // But we should be ready. However, login() is synchronous and waits for response.
        // So we should NOT poll during login.
    } else {
        emit errorOccurred("Failed to connect to server");
    }
}

void NetworkManager::login(const QString &username, const QString &password) {
    if (!m_client->isConnected()) {
        // Try to connect to default if not connected
        if (!m_client->connect()) {
            emit loginFailure("Could not connect to server");
            return;
        }
        emit connectionStatusChanged(true);
    }

    // Stop polling during synchronous login to avoid stealing the response
    m_pollTimer->stop();

    bool success = m_client->login(username.toStdString(), password.toStdString());
    
    if (success) {
        emit loginSuccess();
        emit loginStatusChanged(true);
        m_pollTimer->start(); // Start polling for subsequent messages (chat, notifications)
    } else {
        emit loginFailure("Login failed");
        emit loginStatusChanged(false);
    }
}

void NetworkManager::disconnect() {
    m_pollTimer->stop();
    m_client->disconnect();
    emit connectionStatusChanged(false);
    emit loginStatusChanged(false);
}

bool NetworkManager::isConnected() const {
    return m_client->isConnected();
}

bool NetworkManager::isLoggedIn() const {
    return m_client->isLoggedIn();
}

void NetworkManager::requestLessonList(const QString &topic, const QString &level) {
    if (m_client->requestLessonList(topic.toStdString(), level.toStdString())) {
        // Success
    } else {
        emit errorOccurred("Failed to request lesson list");
    }
}

void NetworkManager::requestStudyLesson(int lessonId, const QString &lessonType) {
    if (m_client->requestStudyLesson(lessonId, lessonType.toStdString())) {
        // Success
    } else {
        emit errorOccurred("Failed to request lesson content");
    }
}

void NetworkManager::requestExerciseList() {
    if (m_client->requestExercises()) {
        // Success
    } else {
        emit errorOccurred("Failed to request exercise list");
    }
}

void NetworkManager::requestExamList() {
    if (m_client->requestExams()) {
        // Success
    } else {
        emit errorOccurred("Failed to request exam list");
    }
}

void NetworkManager::requestResultList() {
    if (m_client->requestResultList()) {
        // Success
    } else {
        emit errorOccurred("Failed to request result list");
    }
}

void NetworkManager::requestExercise(int type, int id) {
    if (m_client->requestExercise(static_cast<protocol::MsgCode>(type), id)) {
        // Success
    } else {
        emit errorOccurred("Failed to request exercise");
    }
}

void NetworkManager::requestExam(int id) {
    if (m_client->requestExam(id)) {
        // Success
    } else {
        emit errorOccurred("Failed to request exam");
    }
}

void NetworkManager::submitAnswer(const QString &targetType, int targetId, const QString &answer) {
    if (m_client->submitAnswer(targetType.toStdString(), targetId, answer.toStdString())) {
        // Success
    } else {
        emit errorOccurred("Failed to submit answer");
    }
}

void NetworkManager::checkMessages() {
    if (!m_client->isConnected()) {
        m_pollTimer->stop();
        return;
    }

    int sockfd = m_client->getSocketFd();
    if (sockfd < 0) return;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0; // Non-blocking check
    
    if (select(sockfd + 1, &readfds, nullptr, nullptr, &timeout) > 0) {
        try {
            protocol::Message msg = m_client->receiveMessage();
            qDebug() << "Received message code:" << (int)msg.code;
            
            switch (msg.code) {
                case protocol::MsgCode::LESSON_LIST_SUCCESS:
                    emit lessonListReceived(QString::fromStdString(msg.toString()));
                    break;
                case protocol::MsgCode::LESSON_LIST_FAILURE:
                    emit errorOccurred("Failed to get lesson list: " + QString::fromStdString(msg.toString()));
                    break;
                case protocol::MsgCode::STUDY_LESSON_SUCCESS:
                    emit lessonContentReceived(QString::fromStdString(msg.toString()));
                    break;
                case protocol::MsgCode::STUDY_LESSON_FAILURE:
                    emit errorOccurred("Failed to get lesson content: " + QString::fromStdString(msg.toString()));
                    break;
                case protocol::MsgCode::EXERCISE_LIST_SUCCESS:
                    emit exerciseListReceived(QString::fromStdString(msg.toString()));
                    break;
                case protocol::MsgCode::EXERCISE_LIST_FAILURE:
                    emit errorOccurred("Failed to get exercise list");
                    break;
                case protocol::MsgCode::EXAM_LIST_SUCCESS:
                    emit examListReceived(QString::fromStdString(msg.toString()));
                    break;
                case protocol::MsgCode::EXAM_LIST_FAILURE:
                    emit errorOccurred("Failed to get exam list");
                    break;
                case protocol::MsgCode::RESULT_LIST_SUCCESS:
                    emit resultListReceived(QString::fromStdString(msg.toString()));
                    break;
                case protocol::MsgCode::RESULT_LIST_FAILURE:
                    emit errorOccurred("Failed to get results");
                    break;
                case protocol::MsgCode::MULTIPLE_CHOICE_SUCCESS:
                case protocol::MsgCode::FILL_IN_SUCCESS:
                case protocol::MsgCode::SENTENCE_ORDER_SUCCESS:
                case protocol::MsgCode::REWRITE_SENTENCE_SUCCESS:
                case protocol::MsgCode::WRITE_PARAGRAPH_SUCCESS:
                case protocol::MsgCode::SPEAKING_TOPIC_SUCCESS:
                    emit exerciseContentReceived(QString::fromStdString(msg.toString()));
                    break;
                case protocol::MsgCode::MULTIPLE_CHOICE_FAILURE:
                case protocol::MsgCode::FILL_IN_FAILURE:
                case protocol::MsgCode::SENTENCE_ORDER_FAILURE:
                case protocol::MsgCode::REWRITE_SENTENCE_FAILURE:
                case protocol::MsgCode::WRITE_PARAGRAPH_FAILURE:
                case protocol::MsgCode::SPEAKING_TOPIC_FAILURE:
                    emit errorOccurred("Failed to get exercise content");
                    break;
                case protocol::MsgCode::EXAM_SUCCESS:
                    emit examContentReceived(QString::fromStdString(msg.toString()));
                    break;
                case protocol::MsgCode::EXAM_FAILURE:
                    emit errorOccurred("Failed to get exam content: " + QString::fromStdString(msg.toString()));
                    break;
                case protocol::MsgCode::SUBMIT_ANSWER_SUCCESS:
                case protocol::MsgCode::SUBMIT_ANSWER_FAILURE:
                    emit answerSubmissionResult(QString::fromStdString(msg.toString()));
                    break;
                default:
                    qDebug() << "Unhandled message code:" << (int)msg.code;
                    break;
            }
            
        } catch (const std::exception& e) {
            qDebug() << "Error receiving message:" << e.what();
        }
    }
}
