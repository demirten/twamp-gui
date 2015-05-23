#ifndef TWAMP_TEST_WORKER_H
#define TWAMP_TEST_WORKER_H

#include <QHostAddress>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimer>
#include <QElapsedTimer>
#include "twamp_common.h"

struct udpTestPacket {
    quint32 index;
    qint64 sent;
    qint64 received;
    quint32 coming_order;
    bool valid;
    float latency;
};

class TwampTestWorker : public TwampCommon
{
    Q_OBJECT
public:
    TwampTestWorker(bool lightMode, QHostAddress destination, int port, unsigned int totalPackets, int interval, int payload);
    ~TwampTestWorker();

private:
    bool lightMode;
    QHostAddress destinationHost;
    int port;
    quint16 receiverUdpPort;
    unsigned int totalPackets;
    int interval;
    int payload;
    enum TestStatus status;

    int padding_length;
    struct twamp_message_test_unauthenticated *message;
    int message_length;
    struct twamp_message_reflector_unathenticated *reflector;
    int reflector_length;
    int received_packets;

    double min_latency;
    double max_latency;
    double average_latency;
    double average_jitter;
    quint32 sequence;

    QList<struct udpTestPacket> packets;
    QUdpSocket *udpSocket;
    QTcpSocket *controlSocket;
    QByteArray controlSocketBuffer;

    /* Timers */
    QTimer *controlHandshakeTimer;
    QTimer *udpSendTimer;
    QTimer *maxTestWaitTimer;
    QElapsedTimer *elapsedTimer;

    double getStdDev(double average);
    void processResults();
    void startControlHandshake();
    void abortControlHandshake(QString message = "Twamp Control Handshake Failed");
    void sendControlSetupResponse();
    void sendControlRequestSession();
    void sendControlStartSessions();
    void sendControlStopSessions();
    void controlHandshakeFinished();

signals:
    void testFinished();
    void testStarted(unsigned int totalPackets);
    void errorMessage(QString);
    void newTestLatency(int, double);
    void packetSent(int index, int totalPackets);
    void twampLogString(QString message);
    void twampLog(int type, QString message, QByteArray data, int status);
    void testResultsCalculated(double packetLoss, double min_latency, double max_latency, double average_latency, double average_jitter);

public slots:
    void doTest();
    void readReflectorMessage();

    void controlSocketConnected();
    void controlSocketError(QAbstractSocket::SocketError error);
    void controlSocketRead();

    void udpSendTimerDone();
    void controlHandshakeTimerDone();
    void maxTestWaitTimerDone();



};

#endif // TWAMP_TEST_WORKER_H
