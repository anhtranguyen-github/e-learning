#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "NetworkManager.h"
#include "common/logger.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    
    // Initialize logger
    logger::initClientLogger("logs/client_gui.log");
    logger::clientLogger->info("Starting Qt Client GUI");
    
    // Create NetworkManager instance
    NetworkManager networkManager;
    
    // Expose to QML
    engine.rootContext()->setContextProperty("networkManager", &networkManager);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    
    // Since we are not using qrc yet (files are on disk), we load from file
    // But for CMake deployment, we copied qml files to binary dir.
    // Let's try loading from local file system for simplicity in dev.
    // Actually, CMakeLists.txt copied them.
    
    QString appDir = QCoreApplication::applicationDirPath();
    engine.load(QUrl::fromLocalFile(appDir + "/main.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
