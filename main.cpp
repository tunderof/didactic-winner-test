#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qqml.h>

#include "src/AppController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<AppController>("test_file_xor", 1, 0, "AppController");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("test_file_xor", "Main");

    return QCoreApplication::exec();
}
