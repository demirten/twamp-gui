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
#include <QMutex>
#include "twamp_common.h"
#include "log_model_data.h"

class Client {
public:
    QTcpSocket *controlSocket;
    QUdpSocket *testSocket;
    QString address;
    quint16 port;
    quint16 testUdpPort;
    qint64 lastUpdate;
    bool setupResponseReceived;
};

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
    struct twamp_time instanceStarted;
    bool running;
    QTimer *removeIdleClientsTimer;

    QElapsedTimer *logTimer;

    QHostAddress destinationHost;

    QTcpServer *controlServer;
    QUdpSocket *udpLightServer;

    QMutex clientMutex;

    void sendGreeting(Client *client);
    void sendServerStart(Client *client);
    void sendAcceptSession(Client *client, quint16 prefferdPort);
    void sendStartSessionsAck(Client *client);


    Client * getClientFromControlSocket(QTcpSocket *socket);
    Client * getClientFromTestSocket(QUdpSocket *socket);
    void removeClient(Client *client);

signals:
    void displayError(QString message);
    void logModelChanged();
    void responderStarted();
    void responderStopped();
    void twampLogString(QString message);
    void twampLog(int type, QString message, QByteArray data, int status);


private slots:
    void acceptNewControlClient();
    void clientRead();
    void clientDisconnected();
    void clientTestPacketRead();
    void twampLightRead();
    void twampLogReceived(int, QString, QByteArray, int);
    void twampLogReceived(QString message);
    void removeIdleClientsTimerDone();

public slots:
    void startServer(int controlPort, int lightPort);
    void stopServer();
    void clearLogs();

};

#endif // TWAMP_RESPONDER_H
