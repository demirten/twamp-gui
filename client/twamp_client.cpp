#include "twamp_client.h"
#include <QDateTime>
#include <QHostInfo>
#include <QThread>

TwampClient::TwampClient()
{
    running = false;

    logTimer = new QElapsedTimer();
    workerThread = NULL;

}

TwampClient::~TwampClient()
{

}

bool TwampClient::startTest(bool lightMode, QString destination, int port, unsigned int totalPackets, int interval, int payload)
{
    if (running) {
        stopTest();
    }
    this->lightMode = lightMode;
    this->destination = destination;
    this->port = port;
    this->totalPackets = totalPackets;
    this->interval = interval;

    QHostAddress address;
    if (address.setAddress(destination)) {
        destinationHost = address;
    } else {
        QHostInfo info = QHostInfo::fromName(destination);
        if (info.error() != QHostInfo::NoError) {
            emit displayError(info.errorString());
            emit testFinished();
            return false;
        }
        destinationHost = info.addresses().first();
    }

    foreach (QObject *obj, mLogModel) {
        obj->deleteLater();
    }
    mLogModel.clear();

    running = true;

    x_values.clear();
    latencies.clear();
    for (unsigned int i = 0; i < totalPackets; i++) {
        x_values.append(QString::number(i));
        latencies.append(0.0);
    }
    emit datasetLatenciesChanged();

    workerThread = new QThread;
    TwampTestWorker* worker = new TwampTestWorker(lightMode, destinationHost, port, totalPackets, interval, payload);
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()), worker, SLOT(doTest()));
    connect(worker, SIGNAL(testFinished()), this, SIGNAL(testFinished()));
    connect(worker, SIGNAL(testStarted(quint32)), this, SIGNAL(testStarted(uint)));
    connect(worker, SIGNAL(newTestLatency(int,double)), this, SLOT(testLatencyReceived(int, double)));
    connect(worker, SIGNAL(errorMessage(QString)), this, SIGNAL(displayError(QString)));
    connect(worker, SIGNAL(packetSent(int,int)), this, SIGNAL(packetSent(int,int)));
    connect(worker, SIGNAL(twampLog(int,QString,QByteArray,int)), this,
            SLOT(twampLogReceived(int,QString,QByteArray,int)));
    connect(worker, SIGNAL(twampLogString(QString)), this, SLOT(twampLogReceived(QString)));
    connect(worker, SIGNAL(testResultsCalculated(double,double,double,double,double)),
            this, SIGNAL(calculatedResults(double,double,double,double,double)));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));
    workerThread->start();

    logTimer->start();
    return true;
}

bool TwampClient::stopTest()
{
    if (running) {
        if (workerThread) {
            workerThread->quit();
            workerThread->deleteLater();
            workerThread = NULL;
        }
    }
    running = false;
    return true;
}

void TwampClient::testLatencyReceived(int index, double latency)
{
    if (index >= latencies.size()) {
        return;
    }
    latencies[index] = latency;
    emit datasetLatenciesChanged();
}

void TwampClient::twampLogReceived(int type, QString message, QByteArray data, int controlType)
{
    double time = logTimer->nsecsElapsed() / 1000000000.0;
    LogModelData *l = new LogModelData(type, message, data, controlType, time);
    mLogModel.append(l);
    emit logModelChanged();
}

void TwampClient::twampLogReceived(QString message)
{
    QByteArray arr;
    return twampLogReceived(TwampLogString, message, arr, StatusUnknown);
}

QList<double> TwampClient::datasetLatencies()
{
    return latencies;
}

QList<QString> TwampClient::xValues()
{
    return x_values;
}

QList<QObject*> TwampClient::logModel()
{
    return mLogModel;
}
