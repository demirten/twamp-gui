#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "twamp_responder.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    TwampResponder responder;

    QQmlContext* ctx = engine.rootContext();
    ctx->setContextProperty("responder", (TwampResponder*)&responder);

    engine.load(QUrl(QStringLiteral("qrc:/responder.qml")));

    return app.exec();
}
