#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "twamp_client.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    TwampClient client;

    QQmlContext* ctx = engine.rootContext();
    ctx->setContextProperty("client", (TwampClient*)&client);

    engine.load(QUrl(QStringLiteral("qrc:/client.qml")));

    return app.exec();
}
