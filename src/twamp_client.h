#ifndef TWAMP_CLIENT_H
#define TWAMP_CLIENT_H

#include <QTimer>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QList>
#include "twamp_common.h"
#include "twamp_test_worker.h"
#include "log_model_data.h"

class TwampClient : public TwampCommon
{
    Q_OBJECT
public:
    TwampClient();
    ~TwampClient();
    Q_PROPERTY(QList<double> datasetLatencies READ datasetLatencies NOTIFY datasetLatenciesChanged)
    QList<double> datasetLatencies();
    Q_PROPERTY(QList<QString> xValues READ xValues NOTIFY xValuesChanged)
    QList<QString> xValues();
    Q_PROPERTY(QList<QObject*> logModel READ logModel NOTIFY logModelChanged)
    QList<QObject*> logModel();

private:
    QList<double> latencies;
    QList<QString> x_values;
    QList<QObject*> mLogModel;
    QString destination;
    int port;
    unsigned int totalPackets;
    int interval;
    bool lightMode;
    int payload;
    bool running;

    QElapsedTimer *logTimer;

    QHostAddress destinationHost;

    QThread *workerThread;

signals:
    void testStarted(unsigned int totalPackets);
    void testFinished();
    void packetSent(int index, int totalPackets);
    void calculatedResults(double packetLoss, double minLatency, double maxLatency, double averageLatency, double averageJitter);
    void displayError(QString message);
    void datasetLatenciesChanged();
    void xValuesChanged();
    void logModelChanged();

private slots:
    void testLatencyReceived(int index, double latency);
    void twampLogReceived(int, QString, QByteArray, int);
    void twampLogReceived(QString message);

public slots:
    bool startTest(bool lightMode, QString destination, int port, unsigned int totalPackets, int interval, int payload);
    bool stopTest();

};

#endif // TWAMP_CLIENT_H
