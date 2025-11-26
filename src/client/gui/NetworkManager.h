#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <memory>
#include "client/network.h"

class NetworkManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY loginStatusChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    Q_INVOKABLE void connectToServer(const QString &host, int port);
    Q_INVOKABLE void login(const QString &username, const QString &password);
    Q_INVOKABLE void disconnect();

    Q_INVOKABLE void requestLessonList(const QString &topic = "", const QString &level = "");
    Q_INVOKABLE void requestStudyLesson(int lessonId, const QString &lessonType);
    Q_INVOKABLE void requestExerciseList();
    Q_INVOKABLE void requestExamList();
    Q_INVOKABLE void requestResultList();
    Q_INVOKABLE void requestExercise(int type, int id);
    Q_INVOKABLE void submitAnswer(const QString &targetType, int targetId, const QString &answer);

    bool isConnected() const;
    bool isLoggedIn() const;

signals:
    void connectionStatusChanged(bool connected);
    void loginStatusChanged(bool loggedIn);
    void loginSuccess();
    void loginFailure(const QString &message);
    void errorOccurred(const QString &message);
    
    void lessonListReceived(const QString &listData);
    void lessonContentReceived(const QString &content);
    void exerciseListReceived(const QString &listData);
    void examListReceived(const QString &listData);
    void resultListReceived(const QString &listData);
    void exerciseContentReceived(const QString &content);
    void answerSubmissionResult(const QString &response);

private slots:
    void checkMessages();

private:
    std::unique_ptr<client::NetworkClient> m_client;
    QTimer *m_pollTimer;
};

#endif // NETWORKMANAGER_H
