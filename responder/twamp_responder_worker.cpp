#include "twamp_responder_worker.h"
#include <QDateTime>
#include <QHostInfo>
#include <QRandomGenerator>

#define MIN_UDP_PORT 8000
#define MAX_UDP_PORT 65000

TwampResponderWorker::TwampResponderWorker(int controlPort, int lightPort)
{
    running = false;
    mStartErrors = "";

    this->controlPort = controlPort;
    this->lightPort   = lightPort;

    controlServer = NULL;
    udpLightServer = NULL;

    removeIdleClientsTimer = new QTimer(this);
    connect(removeIdleClientsTimer, SIGNAL(timeout()), this, SLOT(removeIdleClientsTimerDone()));
    removeIdleClientsTimer->start(10 * 1000);

    /* bind control tcp socket server */
    controlServer = new QTcpServer(this);
    if (!controlServer->listen(QHostAddress::Any, controlPort)) {
        mStartErrors = "Couldn't listen to tcp port: " + QString::number(controlPort);
        if (controlPort < 1024) {
            mStartErrors += "\nListening ports below <1024 requires admin privileges";
        }
        return;
    }
    udpLightServer = new QUdpSocket(this);
    if (!udpLightServer->bind(QHostAddress::Any, lightPort)) {
        mStartErrors = "Couldn't listen to udp port: " + QString::number(lightPort);
        if (lightPort < 1024) {
            mStartErrors += "\nListening ports below <1024 requires admin privileges";
        }
        return;
    }
}

TwampResponderWorker::~TwampResponderWorker()
{
    stopServer();
}

QString TwampResponderWorker::startErrors()
{
    return mStartErrors;
}

void TwampResponderWorker::startServer()
{
    instanceStarted = getTwampTime();
    emit twampLogString("Responder started");
    connect(controlServer, SIGNAL(newConnection()), this, SLOT(acceptNewControlClient()));
    connect(udpLightServer, SIGNAL(readyRead()), this, SLOT(twampLightRead()));
    emit responderStarted();
}

void TwampResponderWorker::stopServer()
{
    if (controlServer == NULL && udpLightServer == NULL) {
        return;
    }
    if (controlServer) {
        controlServer->disconnect();
        controlServer->deleteLater();
        controlServer = NULL;
    }
    if (udpLightServer) {
        udpLightServer->disconnect();
        udpLightServer->deleteLater();
        udpLightServer = NULL;
    }
    QMutexLocker locker(&clientMutex);
    foreach (Client client, mClients) {
        stopClient(&client);
    }
    mClients.clear();
    emit twampLogString("Responder stopped");
    emit responderStopped();
}

void TwampResponderWorker::acceptNewControlClient()
{
    QTcpSocket *socket = controlServer->nextPendingConnection();
    Client client;
    client.controlSocket = socket;
    client.testSocket = NULL;
    client.address = socket->peerAddress().toString();
    client.port = socket->peerPort();
    client.lastUpdate = QDateTime::currentMSecsSinceEpoch();
    client.setupResponseReceived = false;
    mClients.append(client);

    connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(clientRead()));

    emit twampLogString("[Control-TCP] New client accepted from: " +
                        client.address + ":" +
                        QString::number(client.port));
    sendGreeting(&client);
}

void TwampResponderWorker::clientDisconnected()
{
    QTcpSocket *socket = (QTcpSocket*) sender();
    Client *client = getClientFromControlSocket(socket);
    if (client == NULL) {
        emit twampLogString("Couldn't get client from control socket: " +
                            socket->peerAddress().toString() + ":" +
                            QString::number(socket->peerPort()));
        return;
    }
    emit twampLogString("Client disconnected: " + client->address + ":" + QString::number(client->port));
    removeClient(client);
}

void TwampResponderWorker::clientRead()
{
    QTcpSocket *socket = (QTcpSocket*) sender();
    QByteArray response = socket->readAll();

    Client *client = getClientFromControlSocket(socket);
    if (client == NULL) {
        emit twampLogString("Couldn't get client from control socket: " +
                            socket->peerAddress().toString() + ":" +
                            QString::number(socket->peerPort()));
        socket->close();
        socket->disconnect();
        return;
    }

    if (!client->setupResponseReceived) {
        if (response.length() < (int) sizeof(struct twamp_message_setup_response)) {
            emit twampLog(TwampLogPacket, "Received short Setup-Response", response, HandshakeError);
        } else {
            emit twampLog(TwampLogPacket, "", response, HandshakeSetupResponse);
            client->setupResponseReceived = true;
            sendServerStart(client);
        }
    } else {
        quint8 type = *(quint8*)response.constData();
        switch (type) {
        case TWAMP_CONTROL_PROTOCOL_PACKET_TYPE_REQUEST_SESSION:
            if (response.length() < (int) sizeof(struct twamp_message_request_session)) {
                emit twampLog(TwampLogPacket, "Received short Request-Sessions", response, HandshakeError);
            } else {
                emit twampLog(TwampLogPacket, "", response, HandshakeRequestSession);
                struct twamp_message_request_session *session = (struct twamp_message_request_session*)response.constData();
                if (session->packets != 0) {
                    emit twampLogString("Number of Packets must be zero in TwampControl");
                    return;
                }
                client->testUdpPort = qFromBigEndian(session->sender_port);
                sendAcceptSession(client, qFromBigEndian(session->receiver_port));
            }
            break;
        case TWAMP_CONTROL_PROTOCOL_PACKET_TYPE_START_SESSIONS:
            if (response.length() < (int) sizeof(struct twamp_message_start_sessions)) {
                emit twampLog(TwampLogPacket, "Received short Start-Sessions", response, HandshakeError);
            } else {
                emit twampLog(TwampLogPacket, "", response, HandshakeStartSession);
                sendStartSessionsAck(client);
            }
            break;
        case TWAMP_CONTROL_PROTOCOL_PACKET_TYPE_STOP_SESSIONS:
            if (response.length() < (int) sizeof(struct twamp_message_stop_sessions)) {
                emit twampLog(TwampLogPacket, "Received short Stop-Sessions", response, HandshakeError);
            } else {
                emit twampLog(TwampLogPacket, "", response, HandshakeStopSession);
                socket->close();
            }
            break;
        default:
            emit twampLogString("Uknown Twamp Protocol Sub-Type Received: " + QString::number(type));
            socket->close();
            break;
        }
    }
}

void TwampResponderWorker::removeIdleClientsTimerDone()
{
    QList<int> willRemove;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    Client client;

    QMutexLocker locker(&clientMutex);

    for (int i = 0; i < mClients.size(); i++) {
        client = mClients[i];
        if (client.lastUpdate > 0 && ((now - client.lastUpdate) > 15 * 1000)) {
            if (client.controlSocket) {
                emit twampLogString("Closing IDLE connection from " + client.address + ":" + QString::number(client.port));
                client.controlSocket->disconnect();
                client.controlSocket->close();
                client.controlSocket->deleteLater();
            }
            if (client.testSocket) {
                client.testSocket->disconnect();
                client.testSocket->close();
                client.testSocket->deleteLater();
            }
            willRemove.push_front(i);
        }
    }
    foreach (int index, willRemove) {
        mClients.removeAt(index);
    }
}

void TwampResponderWorker::sendGreeting(Client *client)
{
    if (client == NULL || client->controlSocket == NULL) {
        return;
    }
    qint64 random;
    int random1;
    int random2;
    struct twamp_message_server_greeting greeting;
    memset(&greeting, 0, sizeof(greeting));

    qToBigEndian(TWAMP_CONTROL_MODE_OPEN, (uchar*)&greeting.modes);

    random = QDateTime::currentMSecsSinceEpoch();
    memcpy(&greeting.challange, &random, 8);
    random = QDateTime::currentMSecsSinceEpoch();
    memcpy(&greeting.challange[8], &random, 8);
    random = QDateTime::currentMSecsSinceEpoch();
    memcpy(&greeting.salt, &random, 8);
    random1 = QRandomGenerator::global()->generate();
    random2 = QRandomGenerator::global()->generate();
    memcpy(&greeting.salt[8], &random1, 4);
    memcpy(&greeting.salt[12], &random2, 4);

    greeting.count = (1 << 12);

    client->controlSocket->write((const char*)&greeting, sizeof(greeting));
    emit twampLog(TwampLogPacket, "", QByteArray((const char*)&greeting, sizeof(greeting)), HandshakeServerGreeting);
}

void TwampResponderWorker::sendServerStart(Client *client)
{
    if (client == NULL || client->controlSocket == NULL) {
        return;
    }
    struct twamp_message_server_start start;
    memset(&start, 0, sizeof(start));
    start.accept = 0;
    qToBigEndian(instanceStarted.seconds, (uchar*)&start.start_time.seconds);
    qToBigEndian(instanceStarted.fraction, (uchar*)&start.start_time.fraction);

    client->controlSocket->write((const char*)&start, sizeof(start));
    emit twampLog(TwampLogPacket, "", QByteArray((const char*)&start, sizeof(start)), HandshakeServerStart);
}

void TwampResponderWorker::sendAcceptSession(Client *client, quint16 prefferedPort)
{
    if (client == NULL || client->controlSocket == NULL) {
        return;
    }
    if (client->testSocket) {
        client->testSocket->close();
        client->testSocket->deleteLater();
        client->testSocket = NULL;
    }

    QUdpSocket *socket = new QUdpSocket(this);
    if (!socket->bind(QHostAddress::Any, prefferedPort)) {
        for (int i = MIN_UDP_PORT; i < MAX_UDP_PORT; i++) {
            if (socket->bind(QHostAddress::Any, i)) {
                break;
            }
        }
    }
    client->testSocket = socket;

    struct twamp_message_accept_session accept;
    memset(&accept, 0, sizeof(accept));
    accept.accept = 0;
    qToBigEndian(socket->localPort(), (uchar*)&accept.port);
    for (int i = 0; i < 16; i++) {
        accept.sid.sid[i] = QRandomGenerator::global()->generate() & 0xFF;
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(clientTestPacketRead()));

    client->controlSocket->write((const char*)&accept, sizeof(accept));
    emit twampLog(TwampLogPacket, "", QByteArray((const char*)&accept, sizeof(accept)), HandshakeAcceptSession);
}

void TwampResponderWorker::sendStartSessionsAck(Client *client)
{
    struct twamp_message_start_ack ack;
    memset(&ack, 0, sizeof(ack));
    ack.accept = 0;

    client->controlSocket->write((const char*)&ack, sizeof(ack));
    emit twampLog(TwampLogPacket, "", QByteArray((const char*)&ack, sizeof(ack)), HandshakeStartSessionAck);
}

Client * TwampResponderWorker::getClientFromControlSocket(QTcpSocket *socket)
{
    Client *c;
    QMutexLocker locker(&clientMutex);
    for (int i = 0; i < mClients.size(); i++) {
        c = &mClients[i];
        if (c->controlSocket == socket) {
            return c;
        }
    }
    return NULL;
}

Client * TwampResponderWorker::getClientFromTestSocket(QUdpSocket *socket)
{
    Client *c;
    QMutexLocker locker(&clientMutex);
    for (int i = 0; i < mClients.size(); i++) {
        c = &mClients[i];
        if (c->testSocket == socket) {
            return c;
        }
    }
    return NULL;
}

void TwampResponderWorker::stopClient(Client *client)
{
    if (client->controlSocket) {
        client->controlSocket->disconnect();
        client->controlSocket->close();
        client->controlSocket->deleteLater();
        client->controlSocket = NULL;
    }
    if (client->testSocket) {
        client->testSocket->disconnect();
        client->testSocket->close();
        client->testSocket->deleteLater();
        client->testSocket = NULL;
    }
}

void TwampResponderWorker::removeClient(Client *client)
{
    const Client *c;
    QMutexLocker locker(&clientMutex);

    stopClient(client);

    for (int i = 0; i < mClients.size(); i++) {
        c = &mClients.at(i);
        if (c == client) {
            mClients.removeAt(i);
            break;
        }
    }
}

void TwampResponderWorker::clientTestPacketRead()
{
    struct twamp_time received = getTwampTime();

    QUdpSocket *socket = (QUdpSocket*) sender();

    char buf[1500];
    int payload = 0;
    int rc = socket->readDatagram(buf, sizeof(buf));
    if (rc > (int)sizeof(struct twamp_message_reflector_unathenticated)) {
        payload = rc - (int)sizeof(struct twamp_message_reflector_unathenticated);
    }

    struct twamp_message_test_unauthenticated *msg = (struct twamp_message_test_unauthenticated*)buf;

    emit twampLog(TwampLogPacket, "", QByteArray((const char*)buf, rc), TestPacketReceived);

    Client *client = getClientFromTestSocket(socket);
    if (client == NULL) {
        emit twampLogString("Couldn't find client info from udp test socket");
        return;
    }

    char outbuf[sizeof(struct twamp_message_reflector_unathenticated) + payload];
    memset(outbuf, 0, sizeof(outbuf));

    struct twamp_message_reflector_unathenticated *response = (struct twamp_message_reflector_unathenticated *) outbuf;

    struct twamp_time reflected = getTwampTime();
    response->sequence_number = msg->sequence_number;
    qToBigEndian(reflected.seconds, (uchar*)&response->timestamp.seconds);
    qToBigEndian(reflected.fraction, (uchar*)&response->timestamp.fraction);
    qToBigEndian(received.seconds, (uchar*)&response->receive_timestamp.seconds);
    qToBigEndian(received.fraction, (uchar*)&response->receive_timestamp.fraction);
    response->sender_sequence_number = msg->sequence_number;
    response->sender_ttl = 255;
    response->sender_timestamp.seconds = msg->timestamp.seconds;
    response->sender_timestamp.fraction = msg->timestamp.fraction;
    response->sender_error_estimate = msg->error_estimate;
    qToBigEndian(TwampCommon::getErrorEstimate(), (uchar*)&response->error_estimate);

    socket->writeDatagram((const char*)outbuf, sizeof(outbuf), client->controlSocket->peerAddress(), client->testUdpPort);
    emit twampLog(TwampLogPacket, "", QByteArray((const char*)outbuf, sizeof(outbuf)), TestPacketSent);
}

void TwampResponderWorker::twampLightRead()
{
    struct twamp_time received = getTwampTime();
    QUdpSocket *socket = (QUdpSocket*) sender();

    char buf[1500];

    QHostAddress peerAddress;
    quint16 peerPort;

    int rc = socket->readDatagram(buf, sizeof(buf), &peerAddress, &peerPort);
    int payload = rc - (int)sizeof(struct twamp_message_test_unauthenticated);

    struct twamp_message_test_unauthenticated *msg = (struct twamp_message_test_unauthenticated*)buf;

    emit twampLog(TwampLogPacket, "", QByteArray((const char*)buf, rc), TestPacketReceived);

    char outbuf[sizeof(struct twamp_message_reflector_unathenticated) + payload];
    memset(outbuf, 0, sizeof(outbuf));

    struct twamp_message_reflector_unathenticated *response = (struct twamp_message_reflector_unathenticated *) outbuf;

    struct twamp_time reflected = getTwampTime();
    response->sequence_number = msg->sequence_number;
    qToBigEndian(reflected.seconds, (uchar*)&response->timestamp.seconds);
    qToBigEndian(reflected.fraction, (uchar*)&response->timestamp.fraction);
    qToBigEndian(received.seconds, (uchar*)&response->receive_timestamp.seconds);
    qToBigEndian(received.fraction, (uchar*)&response->receive_timestamp.fraction);
    response->sender_sequence_number = msg->sequence_number;
    response->sender_timestamp.seconds = msg->timestamp.seconds;
    response->sender_timestamp.fraction = msg->timestamp.fraction;

    socket->writeDatagram((const char*)outbuf, sizeof(outbuf), peerAddress, peerPort);
    emit twampLog(TwampLogPacket, "", QByteArray((const char*)outbuf, sizeof(outbuf)), TestPacketSent);
}
