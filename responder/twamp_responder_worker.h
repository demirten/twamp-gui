#ifndef TWAMP_RESPONDER_WORKER_H
#define TWAMP_RESPONDER_WORKER_H

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

class TwampResponderWorker : public TwampCommon
{
    Q_OBJECT
public:
    TwampResponderWorker(int, int);
    ~TwampResponderWorker();

    QString startErrors();

private:
    QList<Client> mClients;
    int controlPort;
    int lightPort;
    QString mStartErrors;
    struct twamp_time instanceStarted;
    bool running;
    QTimer *removeIdleClientsTimer;

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
    void stopClient(Client *client);

signals:
    void errorMessage(QString message);
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
    void removeIdleClientsTimerDone();

public slots:
    void startServer();
    void stopServer();
};

#endif // TWAMP_RESPONDER_WORKER_H
