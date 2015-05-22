#include "twamp_responder.h"
#include "twamp_responder_worker.h"
#include <QDateTime>
#include <QHostInfo>

#define MIN_UDP_PORT 8000
#define MAX_UDP_PORT 65000

TwampResponder::TwampResponder()
{
    workerThread = NULL;
    logTimer = new QElapsedTimer();
}

TwampResponder::~TwampResponder()
{

}

void TwampResponder::startServer(int controlPort, int lightPort)
{
    clearLogs();
    logTimer->restart();

    TwampResponderWorker *worker = new TwampResponderWorker(controlPort, lightPort);

    if (worker->startErrors().length() > 0) {
        displayError(worker->startErrors());
        worker->deleteLater();
        return;
    }

    workerThread = new QThread;
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()), worker, SLOT(startServer()));
    connect(worker, SIGNAL(errorMessage(QString)), this, SIGNAL(displayError(QString)));
    connect(worker, SIGNAL(twampLog(int,QString,QByteArray,int)), this,
            SLOT(twampLogReceived(int,QString,QByteArray,int)));
    connect(worker, SIGNAL(twampLogString(QString)), this, SLOT(twampLogReceived(QString)));
    connect(worker, SIGNAL(responderStarted()), this, SIGNAL(responderStarted()));
    connect(worker, SIGNAL(responderStopped()), this, SIGNAL(responderStopped()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));
    workerThread->start();
}

void TwampResponder::stopServer()
{
    if (workerThread) {
        workerThread->quit();
        workerThread = NULL;
    }
    emit responderStopped();
}

QList<QObject*> TwampResponder::logModel()
{
    return mLogModel;
}

void TwampResponder::twampLogReceived(int type, QString message, QByteArray data, int controlType)
{
    double time = logTimer->nsecsElapsed() / 1000000000.0;
    LogModelData *l = new LogModelData(type, message, data, controlType, time);
    mLogModel.append(l);
    emit logModelChanged();
}

void TwampResponder::twampLogReceived(QString message)
{
    QByteArray arr;
    return twampLogReceived(TwampLogString, message, arr, StatusUnknown);
}

void TwampResponder::clearLogs()
{
    foreach (QObject *obj, mLogModel) {
        obj->deleteLater();
    }
    mLogModel.clear();
    emit logModelChanged();
}
