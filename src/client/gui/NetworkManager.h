#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QAudioRecorder>
#include <memory>
#include "client/network.h"

class NetworkManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY loginStatusChanged)
    Q_PROPERTY(QString userRole READ userRole NOTIFY userRoleChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    Q_INVOKABLE void connectToServer(const QString &host, int port);
    Q_INVOKABLE void login(const QString &username, const QString &password);
    Q_INVOKABLE void registerUser(const QString &username, const QString &password);
    Q_INVOKABLE void disconnect();

    Q_INVOKABLE void requestLessonList(const QString &topic = "", const QString &level = "");
    Q_INVOKABLE void requestStudyLesson(int lessonId, const QString &lessonType);
    Q_INVOKABLE void requestExerciseList();
    Q_INVOKABLE void requestExamList();
    Q_INVOKABLE void requestResultList();
    Q_INVOKABLE void requestExercise(int type, int id);
    Q_INVOKABLE void requestExam(int id);
    Q_INVOKABLE void requestExamReview(int id);  // Teacher exam review
    Q_INVOKABLE void submitAnswer(const QString &targetType, int targetId, const QString &answer);
    Q_INVOKABLE QString createAudioRecordingPath();
    Q_INVOKABLE QString saveAudioFromBase64(const QString &base64);
    Q_INVOKABLE QString startSpeakingRecording();
    Q_INVOKABLE QString stopSpeakingRecording();
    Q_INVOKABLE void requestPendingSubmissions();
    Q_INVOKABLE void requestResultDetail(const QString &targetType, const QString &targetId);
    Q_INVOKABLE void submitGrade(const QString &resultId, const QString &score, const QString &feedback, const QString &gradingDetails = "{}");
    Q_INVOKABLE void sendPrivateMessage(const QString &recipient, const QString &content, const QString &type = "TEXT");
    Q_INVOKABLE void requestChatHistory(const QString &otherUser);
    Q_INVOKABLE void requestRecentChats();

    // Voice Calls
    Q_INVOKABLE void initiateCall(const QString &targetUser);
    Q_INVOKABLE void answerCall(const QString &callerUser);
    Q_INVOKABLE void declineCall(const QString &callerUser);
    Q_INVOKABLE void endCall(const QString &otherUser);

    Q_INVOKABLE void requestGameList();
    Q_INVOKABLE void requestGameLevelList(const QString &gameType);
    Q_INVOKABLE void requestGameData(const QString &gameId);
    Q_INVOKABLE void submitGameResult(const QString &gameId, const QString &score, const QString &detailsJson);

    // Admin Game Management
    Q_INVOKABLE void requestAdminCreateGame(const QString &type, const QString &level, const QString &questionJson);
    Q_INVOKABLE void requestAdminUpdateGame(const QString &gameId, const QString &type, const QString &level, const QString &questionJson);
    Q_INVOKABLE void requestAdminDeleteGame(const QString &gameId);

    bool isConnected() const;
    bool isLoggedIn() const;
    QString userRole() const;

signals:
    void connectionStatusChanged(bool connected);
    void loginStatusChanged(bool loggedIn);
    void userRoleChanged(const QString &role);
    void loginSuccess();
    void loginFailure(const QString &message);
    void registerSuccess();
    void registerFailure(const QString &message);
    void errorOccurred(const QString &message);
    
    void lessonListReceived(const QString &listData);
    void lessonContentReceived(const QString &content);
    void exerciseListReceived(const QString &listData);
    void examListReceived(const QString &listData);
    void resultListReceived(const QString &listData);
    void resultDetailReceived(const QString &detailData);
    void exerciseContentReceived(const QString &content);
    void examContentReceived(const QString &content);
    void answerSubmissionResult(const QString &response);
    void answerSubmissionSuccess(const QString &targetType, const QString &targetId);
    void answerSubmissionFailure(const QString &message);
    void pendingSubmissionsReceived(const QString &listData);
    void gradeSubmissionSuccess(const QString &message);
    void gradeSubmissionFailure(const QString &message);
    void examAlreadyTaken(const QString &message);
    
    void chatMessageReceived(const QString &sender, const QString &content, const QString &type, const QString &timestamp);
    void chatHistoryReceived(const QString &historyData);
    void recentChatsReceived(const QString &chatsData);
    void chatMessageSent(const QString &message);
    void chatError(const QString &message);

    // Voice Call Signals
    void incomingCall(const QString &callerUsername, const QString &callerId);
    void callAnswered(const QString &username);
    void callEnded(const QString &reason);
    void callFailed(const QString &reason);

    // Game Signals
    void gameListReceived(const QString &listData);
    void gameLevelListReceived(const QString &listData);
    void gameDataReceived(const QString &data);
    void gameSubmitSuccess(const QString &message);
    void gameSubmitFailure(const QString &message);
    
    // Admin Game Signals
    void gameCreateSuccess(const QString &message);
    void gameCreateFailure(const QString &message);
    void gameUpdateSuccess(const QString &message);
    void gameUpdateFailure(const QString &message);
    void gameDeleteSuccess(const QString &message);
    void gameDeleteFailure(const QString &message);

private slots:
    void checkMessages();

private:
    std::unique_ptr<client::NetworkClient> m_client;
    QTimer *m_pollTimer;
    QAudioRecorder *m_audioRecorder;
    QString m_audioOutputPath;
};

#endif // NETWORKMANAGER_H
