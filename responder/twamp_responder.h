#ifndef TWAMP_RESPONDER_H
#define TWAMP_RESPONDER_H

#include <QTimer>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QThread>
#include <QMutex>
#include "twamp_common.h"
#include "twamp_responder_worker.h"
#include "log_model_data.h"


class TwampResponder : public TwampCommon
{
    Q_OBJECT
public:
    TwampResponder();
    ~TwampResponder();
    Q_PROPERTY(QList<QObject*> logModel READ logModel NOTIFY logModelChanged)
    QList<QObject*> logModel();

private:
    QList<QObject*> mLogModel;
    QList<Client> mClients;
    int controlPort;
    int lightPort;
    bool running;
    QThread *workerThread;
    QElapsedTimer *logTimer;

signals:
    void displayError(QString message);
    void logModelChanged();
    void twampLogString(QString message);
    void twampLog(int type, QString message, QByteArray data, int status);
    void responderStarted();
    void responderStopped();


private slots:
    void twampLogReceived(int, QString, QByteArray, int);
    void twampLogReceived(QString message);

public slots:
    void startServer(int controlPort, int lightPort, bool collectLogs);
    void stopServer();
    void clearLogs();

};

#endif // TWAMP_RESPONDER_H
