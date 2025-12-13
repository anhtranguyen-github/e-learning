#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "NetworkManager.h"
#include "common/logger.h"
#include <csignal>

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    

    logger::initClientLogger("logs/client_gui.log");
    logger::initMessageLogger("logs/client_msg.log");
    logger::clientLogger->info("Starting Qt Client GUI");
    

    NetworkManager networkManager;
    
    // Expose to QML
    engine.rootContext()->setContextProperty("networkManager", &networkManager);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    

    QString appDir = QCoreApplication::applicationDirPath();
    engine.load(QUrl::fromLocalFile(appDir + "/main.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
