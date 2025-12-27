#include "NetworkManager.h"
#include "common/payloads.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDateTime>
#include <QCryptographicHash>
#include <QUrl>
#include <QAudioEncoderSettings>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent), m_client(std::make_unique<client::NetworkClient>()) {
    
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(100); // Poll every 100ms
    connect(m_pollTimer, &QTimer::timeout, this, &NetworkManager::checkMessages);

    m_audioRecorder = new QAudioRecorder(this);
    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/pcm");
    audioSettings.setSampleRate(44100);
    audioSettings.setChannelCount(1);
    audioSettings.setQuality(QMultimedia::HighQuality);
    m_audioRecorder->setAudioSettings(audioSettings);
    m_audioRecorder->setContainerFormat("wav");
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
        emit userRoleChanged(userRole());
        m_pollTimer->start(); // Start polling for subsequent messages (chat, notifications)
    } else {
        emit loginFailure("Login failed");
        emit loginStatusChanged(false);
    }
}

void NetworkManager::registerUser(const QString &username, const QString &password) {
    if (!m_client->isConnected()) {
        // Try to connect to default if not connected
        if (!m_client->connect()) {
            emit registerFailure("Could not connect to server");
            return;
        }
        emit connectionStatusChanged(true);
    }

    bool success = m_client->registerUser(username.toStdString(), password.toStdString());
    
    if (success) {
        emit registerSuccess();
    } else {
        emit registerFailure("Registration failed");
    }
}

void NetworkManager::disconnect() {
    m_pollTimer->stop();
    m_client->disconnect();
    emit connectionStatusChanged(false);
    emit loginStatusChanged(false);
    emit userRoleChanged("");
}

bool NetworkManager::isConnected() const {
    return m_client->isConnected();
}

bool NetworkManager::isLoggedIn() const {
    return m_client->isLoggedIn();
}

QString NetworkManager::userRole() const {
    return QString::fromStdString(m_client->getUserRole());
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
    requestExercise(type, id, "");
}

void NetworkManager::requestExercise(int type, int id, const QString &studentId) {
    if (m_client->requestExercise(static_cast<protocol::MsgCode>(type), id, studentId.toStdString())) {
        // Success
    } else {
        emit errorOccurred("Failed to request exercise");
    }
}

void NetworkManager::requestSubmissionDetail(const QString &resultId) {
    if (m_client->requestSubmissionDetail(resultId.toStdString())) {
        // Success
    } else {
        emit errorOccurred("Failed to request submission detail");
    }
}

void NetworkManager::requestExam(int id) {
    if (m_client->requestExam(id)) {
        // Success
    } else {
        emit errorOccurred("Failed to request exam");
    }
}

void NetworkManager::requestExamReview(int id) {
    if (m_client->requestExamReview(id)) {
        // Success
    } else {
        emit errorOccurred("Failed to request exam review");
    }
}

void NetworkManager::submitAnswer(const QString &targetType, int targetId, const QString &answer) {
    std::string finalAnswer = answer.toStdString();
    QFile file(answer);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray fileData = file.readAll();
            finalAnswer = fileData.toBase64().toStdString();
            file.close();
        } else {
            emit errorOccurred("Failed to read audio file");
            return;
        }
    }

    if (m_client->submitAnswer(targetType.toStdString(), targetId, finalAnswer)) {
        // Success
    } else {
        emit errorOccurred("Failed to submit answer");
    }
}

QString NetworkManager::createAudioRecordingPath() {
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (baseDir.isEmpty()) {
        baseDir = QDir::tempPath();
    }
    QDir dir(baseDir + "/socker_recordings");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString fileName = QString("speaking_%1.wav").arg(QDateTime::currentMSecsSinceEpoch());
    return dir.filePath(fileName);
}

QString NetworkManager::saveAudioFromBase64(const QString &base64) {
    if (base64.isEmpty()) {
        return "";
    }

    QString trimmed = base64;
    int marker = trimmed.indexOf("base64,");
    if (marker != -1) {
        trimmed = trimmed.mid(marker + 7);
    }

    QByteArray audioData = QByteArray::fromBase64(trimmed.toUtf8());
    if (audioData.isEmpty()) {
        return "";
    }

    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (baseDir.isEmpty()) {
        baseDir = QDir::tempPath();
    }
    QDir dir(baseDir + "/socker_recordings");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString fileName = QString("speaking_review_%1.wav").arg(QDateTime::currentMSecsSinceEpoch());
    QString filePath = dir.filePath(fileName);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return "";
    }
    file.write(audioData);
    file.close();
    return filePath;
}

QString NetworkManager::startSpeakingRecording() {
    if (!m_audioRecorder) {
        return "";
    }

    if (m_audioRecorder->state() == QMediaRecorder::RecordingState) {
        return m_audioOutputPath;
    }

    m_audioOutputPath = createAudioRecordingPath();
    m_audioRecorder->setOutputLocation(QUrl::fromLocalFile(m_audioOutputPath));
    m_audioRecorder->record();
    return m_audioOutputPath;
}

QString NetworkManager::stopSpeakingRecording() {
    if (!m_audioRecorder) {
        return "";
    }

    if (m_audioRecorder->state() == QMediaRecorder::RecordingState) {
        m_audioRecorder->stop();
    }

    return m_audioOutputPath;
}

void NetworkManager::requestPendingSubmissions() {
    if (m_client->requestPendingSubmissions()) {
        // Success
    } else {
        emit errorOccurred("Failed to request pending submissions");
    }
}

void NetworkManager::submitGrade(const QString &resultId, const QString &userId, const QString &score, const QString &feedback) {
    if (m_client->submitGrade(resultId.toStdString(), userId.toStdString(), score.toStdString(), feedback.toStdString())) {
        // Success
    } else {
        emit errorOccurred("Failed to submit grade");
    }
}



void NetworkManager::requestResultDetail(const QString &targetType, const QString &targetId) {
    if (m_client->requestResultDetail(targetType.toStdString(), targetId.toStdString())) {
        // Success
    } else {
        emit errorOccurred("Failed to request result detail");
    }
}

void NetworkManager::sendPrivateMessage(const QString &recipient, const QString &content, const QString &type) {
    // If type is AUDIO, content is expected to be Base64 encoded audio data
    // We don't need to do anything special here, just pass it.
    // But if the UI passes a file path, we might need to read and encode it?
    // The requirement says "Implement audio recording and Base64 encoding/decoding (Client)".
    // If the UI (QML) records audio, it likely saves to a file.
    // So `content` might be a file path.
    // If `type` is AUDIO and `content` looks like a file path, read and encode it.
    
    std::string finalContent = content.toStdString();
    
    if (type == "AUDIO") {
        QFile file(content);
        if (file.exists()) {
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray fileData = file.readAll();
                finalContent = fileData.toBase64().toStdString();
                file.close();
            } else {
                emit chatError("Failed to read audio file");
                return;
            }
        } else {
            // Assume it's already base64 or raw content if not a file
        }
    }

    if (m_client->sendPrivateMessage(recipient.toStdString(), finalContent, type.toStdString())) {
        emit chatMessageSent(content); // Emit original content (e.g. file path or text) so UI can show it
    } else {
        emit chatError("Failed to send message");
    }
}

void NetworkManager::requestChatHistory(const QString &otherUser) {
    if (m_client->requestChatHistory(otherUser.toStdString())) {
        // Success
    } else {
        emit chatError("Failed to request chat history");
    }
}

void NetworkManager::requestRecentChats() {
    if (m_client->requestRecentChats()) {
        // Success
    } else {
        emit chatError("Failed to request recent chats");
    }
}

void NetworkManager::initiateCall(const QString &targetUser) {
    if (!m_client->initiateCall(targetUser.toStdString())) {
        emit callFailed("Failed to initiate call");
    }
}

void NetworkManager::answerCall(const QString &callerUser) {
    m_client->answerCall(callerUser.toStdString());
}

void NetworkManager::declineCall(const QString &callerUser) {
    m_client->declineCall(callerUser.toStdString());
}

void NetworkManager::endCall(const QString &otherUser) {
    m_client->endCall(otherUser.toStdString());
}

// Game Methods
// Game Methods
void NetworkManager::requestGameList() {
   if (m_client->requestGameList()) {
       // Success
   } else {
       emit errorOccurred("Failed to request game list");
   }
}

void NetworkManager::requestGameLevelList(const QString &gameType) {
    if (m_client->requestGameLevelList(gameType.toStdString())) {
        // Success
    } else {
        emit errorOccurred("Failed to request game levels");
    }
}

void NetworkManager::requestGameData(const QString &gameId) {
    if (m_client->requestGameData(gameId.toStdString())) {
        // Success
    } else {
        emit errorOccurred("Failed to request game data");
    }
}

void NetworkManager::submitGameResult(const QString &gameId, const QString &score, const QString &detailsJson) {
    if (m_client->submitGameResult(gameId.toStdString(), score.toStdString(), detailsJson.toStdString())) {
        // Success
    } else {
        emit gameSubmitFailure("Failed to submit game result");
    }
}

void NetworkManager::requestAdminCreateGame(const QString &type, const QString &level, const QString &questionJson) {
    if (m_client->requestCreateGame(type.toStdString(), level.toStdString(), questionJson.toStdString())) {
        // Success
    } else {
        emit gameCreateFailure("Failed to send create game request");
    }
}

void NetworkManager::requestAdminUpdateGame(const QString &gameId, const QString &type, const QString &level, const QString &questionJson) {
    if (m_client->requestUpdateGame(gameId.toStdString(), type.toStdString(), level.toStdString(), questionJson.toStdString())) {
        // Success
    } else {
        emit gameUpdateFailure("Failed to send update game request");
    }
}

void NetworkManager::requestAdminDeleteGame(const QString &gameId) {
    if (m_client->requestDeleteGame(gameId.toStdString())) {
        // Success
    } else {
        emit gameDeleteFailure("Failed to send delete game request");
    }
}


void NetworkManager::checkMessages() {
    if (!m_client->isConnected()) {
        m_pollTimer->stop();
        return;
    }

    std::vector<protocol::Message> messages = m_client->pollMessages();
    
    if (!m_client->isConnected()) {
        m_pollTimer->stop();
        emit connectionStatusChanged(false);
        emit loginStatusChanged(false); // Also update login status
        emit errorOccurred("Disconnected from server");
        return;
    }
    
    for (const auto& msg : messages) {
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
            case protocol::MsgCode::RESULT_DETAIL_SUCCESS:
                emit resultDetailReceived(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::RESULT_DETAIL_FAILURE: // Assuming this exists or falls back to generic failure
                 emit errorOccurred("Failed to get result detail");
                 break;
            case protocol::MsgCode::STUDY_EXERCISE_SUCCESS:
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
            case protocol::MsgCode::EXAM_ALREADY_TAKEN:
                emit examAlreadyTaken(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::SUBMIT_ANSWER_SUCCESS: {
                // Parse response to get targetType and targetId if possible, or just emit success
                // The payload is ResultDTO: score|feedback
                // Wait, we need targetType and targetId to redirect.
                // The server response currently only contains score and feedback.
                // We need to store the last submitted targetType and targetId in NetworkManager or pass it back.
                // Or we can just emit success and let the UI handle it (since UI knows what it submitted).
                emit answerSubmissionResult(QString::fromStdString(msg.toString()));
                emit answerSubmissionSuccess("", ""); // UI will use its own stored values
                break;
            }
            case protocol::MsgCode::SUBMIT_ANSWER_FAILURE:
                emit answerSubmissionResult(QString::fromStdString(msg.toString()));
                emit answerSubmissionFailure(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::PENDING_SUBMISSIONS_SUCCESS:
                // Reusing RESULT_LIST_SUCCESS logic or separate signal?
                // In protocol.h I reused RESULT_LIST_SUCCESS for pending submissions response in controller
                // Wait, in ResultController::handlePendingSubmissionsRequest I used RESULT_LIST_SUCCESS.
                // So I should handle it here.
                // Actually, I should check if I used RESULT_LIST_SUCCESS or PENDING_SUBMISSIONS_SUCCESS in the controller.
                // In ResultController I used: protocol::Message response(protocol::MsgCode::RESULT_LIST_SUCCESS, responsePayload);
                // So the client will receive RESULT_LIST_SUCCESS.
                // This might be ambiguous if I want to distinguish between "My Results" and "Pending Submissions".
                // Ideally I should have used PENDING_SUBMISSIONS_SUCCESS.
                // Let's check ResultController again.
                // Yes, I used RESULT_LIST_SUCCESS.
                // This is a problem because `resultListReceived` signal is connected to `ResultListScreen`.
                // If `GradingListScreen` also listens to `resultListReceived`, it might get confused.
                // However, `GradingListScreen` calls `requestPendingSubmissions`, so it expects a response.
                // If I change the controller to use PENDING_SUBMISSIONS_SUCCESS, I can emit `pendingSubmissionsReceived`.
                // I should fix the controller to use PENDING_SUBMISSIONS_SUCCESS.
                // But for now, let's assume I will fix the controller.
                emit pendingSubmissionsReceived(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::PENDING_SUBMISSIONS_FAILURE:
                emit errorOccurred("Failed to get pending submissions");
                break;
            case protocol::MsgCode::SUBMISSION_DETAIL_SUCCESS: {
                Payloads::SubmissionDetailDTO dto;
                dto.deserialize(msg.toString());
                emit submissionDetailReceived(QString::fromStdString(dto.resultId),
                                              QString::fromStdString(dto.userAnswer));
                break;
            }
            case protocol::MsgCode::SUBMISSION_DETAIL_FAILURE:
                emit errorOccurred("Failed to get submission detail");
                break;
            case protocol::MsgCode::GRADE_SUBMISSION_SUCCESS:
                emit gradeSubmissionSuccess(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::GRADE_SUBMISSION_FAILURE:
                emit gradeSubmissionFailure(QString::fromStdString(msg.toString()));
                break;
            
            // Chat
            case protocol::MsgCode::CHAT_MESSAGE_SUCCESS:
                // Message sent successfully
                // We already emitted chatMessageSent optimistically, or we can do it here.
                break;
            case protocol::MsgCode::CHAT_MESSAGE_FAILURE:
                emit chatError("Failed to send message: " + QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::CHAT_PRIVATE_RECEIVE: {
                Payloads::ChatMessageDTO dto;
                dto.deserialize(msg.toString());
                
                QString content = QString::fromStdString(dto.content);
                if (dto.messageType == "AUDIO") {
                    // Save audio to file
                    QByteArray audioData = QByteArray::fromBase64(dto.content.c_str());
                    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
                    QDir dir(downloadsPath);
                    if (!dir.exists()) dir.mkpath(".");
                    
                    QString fileName = QString("audio_%1_%2.wav").arg(QString::fromStdString(dto.sender)).arg(QDateTime::currentMSecsSinceEpoch());
                    QString filePath = dir.filePath(fileName);
                    
                    QFile file(filePath);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(audioData);
                        file.close();
                        content = filePath; // Update content to be the file path
                    } else {
                        qDebug() << "Failed to save audio file:" << filePath;
                    }
                }
                
                emit chatMessageReceived(QString::fromStdString(dto.sender), content, QString::fromStdString(dto.messageType), QString::fromStdString(dto.timestamp));
                break;
            }
            case protocol::MsgCode::CHAT_HISTORY_SUCCESS: {
                Payloads::ChatHistoryDTO historyDto;
                historyDto.deserialize(msg.toString());
                
                // Process history to save audio files if needed
                // We need to reconstruct the DTO with file paths instead of Base64
                // But ChatHistoryDTO contains a vector of ChatMessageDTO.
                // We can iterate and modify, then serialize again? Or just emit the modified list?
                // The signal expects a string (serialized data).
                // If we modify the content, we need to re-serialize.
                
                for (auto& dto : historyDto.messages) {
                    if (dto.messageType == "AUDIO") {
                        // Check if we already have this file? 
                        // Generating a unique name based on timestamp/sender might duplicate if we fetch history multiple times.
                        // Ideally, we should hash the content or use message ID (but DTO doesn't have ID).
                        // For now, let's save it again or check if existing.
                        // To avoid re-saving, we could check if content is already a path? No, it's Base64 from server.
                        
                        QByteArray audioData = QByteArray::fromBase64(dto.content.c_str());
                        QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
                        QDir dir(downloadsPath);
                        if (!dir.exists()) dir.mkpath(".");
                        
                        // Use a hash of content for filename to avoid duplicates
                        QString hash = QString(QCryptographicHash::hash(audioData, QCryptographicHash::Md5).toHex());
                        QString fileName = QString("audio_%1.wav").arg(hash);
                        QString filePath = dir.filePath(fileName);
                        
                        QFile file(filePath);
                        if (!file.exists()) {
                            if (file.open(QIODevice::WriteOnly)) {
                                file.write(audioData);
                                file.close();
                            }
                        }
                        dto.content = filePath.toStdString();
                    }
                }
                
                emit chatHistoryReceived(QString::fromStdString(historyDto.serialize()));
                break;
            }
            case protocol::MsgCode::CHAT_HISTORY_FAILURE:
                emit chatError("Failed to get chat history");
                break;
            case protocol::MsgCode::RECENT_CHATS_SUCCESS:
                emit recentChatsReceived(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::RECENT_CHATS_FAILURE:
                emit chatError("Failed to get recent chats");
                break;
            case protocol::MsgCode::GENERAL_FAILURE:
                emit errorOccurred("Server error: " + QString::fromStdString(msg.toString()));
                break;

            // Voice Calls
            case protocol::MsgCode::CALL_INCOMING: {
                Payloads::VoiceCallNotification notification;
                notification.deserialize(msg.toString());
                emit incomingCall(QString::fromStdString(notification.callerUsername), 
                                  QString::fromStdString(notification.callerId));
                break;
            }
            case protocol::MsgCode::CALL_ANSWER_REQUEST: {
                Payloads::VoiceCallNotification notification;
                notification.deserialize(msg.toString());
                emit callAnswered(QString::fromStdString(notification.callerUsername));
                break;
            }
            case protocol::MsgCode::CALL_ENDED:
                emit callEnded(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::CALL_FAILED:
                emit callFailed(QString::fromStdString(msg.toString()));
                break;

            // Admin Game Management Responses
            case protocol::MsgCode::GAME_CREATE_SUCCESS:
                emit gameCreateSuccess(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::GAME_CREATE_FAILURE:
                emit gameCreateFailure(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::GAME_UPDATE_SUCCESS:
                emit gameUpdateSuccess(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::GAME_UPDATE_FAILURE:
                emit gameUpdateFailure(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::GAME_DELETE_SUCCESS:
                emit gameDeleteSuccess(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::GAME_DELETE_FAILURE:
                emit gameDeleteFailure(QString::fromStdString(msg.toString()));
                break;



            // Game Play Responses
            case protocol::MsgCode::GAME_LIST_SUCCESS:
                emit gameListReceived(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::GAME_LIST_FAILURE:
                emit errorOccurred("Failed to get game list");
                break;
            case protocol::MsgCode::GAME_LEVEL_LIST_SUCCESS:
                emit gameLevelListReceived(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::GAME_LEVEL_LIST_FAILURE:
                emit errorOccurred("Failed to get game levels");
                break;
            case protocol::MsgCode::GAME_DATA_SUCCESS:
                emit gameDataReceived(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::GAME_DATA_FAILURE:
                emit errorOccurred("Failed to get game data");
                break;
            case protocol::MsgCode::GAME_SUBMIT_SUCCESS:
                emit gameSubmitSuccess(QString::fromStdString(msg.toString()));
                break;
            case protocol::MsgCode::GAME_SUBMIT_FAILURE:
                emit gameSubmitFailure(QString::fromStdString(msg.toString()));
                break;

            default:
                qDebug() << "Unhandled message code:" << (int)msg.code;
                break;
        }
    }
}
