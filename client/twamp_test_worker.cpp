#include <QtEndian>
#include <QtMath>
#include "twamp_test_worker.h"

#define MIN_UDP_PORT 40000
#define MAX_UDP_PORT 60000

TwampTestWorker::TwampTestWorker(bool lightMode, QHostAddress destinationHost, int port, unsigned int totalPackets, int interval, int payload)
{
    this->lightMode = lightMode;
    this->destinationHost = destinationHost;
    this->port = port;
    this->totalPackets = totalPackets;
    this->interval = interval;
    this->payload = payload;

    sequence = 0;
    received_packets = 0;
    status = StatusUnknown;

    controlHandshakeTimer = new QTimer(this);
    controlHandshakeTimer->setSingleShot(true);
    connect(controlHandshakeTimer, SIGNAL(timeout()), this, SLOT(controlHandshakeTimerDone()));

    udpSendTimer = new QTimer(this);
    connect(udpSendTimer, SIGNAL(timeout()), this, SLOT(udpSendTimerDone()));

    maxTestWaitTimer = new QTimer(this);
    maxTestWaitTimer->setSingleShot(true);
    connect(maxTestWaitTimer, SIGNAL(timeout()), this, SLOT(maxTestWaitTimerDone()));

    udpSocket = new QUdpSocket(this);
    for (int i = MIN_UDP_PORT; i < MAX_UDP_PORT; i++) {
        if (udpSocket->bind(QHostAddress::Any, i)) {
            break;
        }
    }

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readReflectorMessage()));

    if (payload < (int) sizeof(struct twamp_message_test_unauthenticated)) {
        payload = sizeof(struct twamp_message_test_unauthenticated);
        padding_length = 0;
    } else {
        padding_length = payload - sizeof(struct twamp_message_test_unauthenticated);
    }
    message_length = sizeof(struct twamp_message_test_unauthenticated) + padding_length;
    message = (struct twamp_message_test_unauthenticated*) calloc(1, message_length);

    reflector_length = sizeof(struct twamp_message_reflector_unathenticated) + padding_length;
    reflector = (struct twamp_message_reflector_unathenticated*) calloc(1, reflector_length);

    packets.reserve(totalPackets);
    for (unsigned int i = 0; i < totalPackets; i++) {
        struct udpTestPacket packet;
        packet.valid = false;
        packets.append(packet);
    }

    elapsedTimer = new QElapsedTimer();
}

TwampTestWorker::~TwampTestWorker()
{
    free(message);
    free(reflector);
}

void TwampTestWorker::startControlHandshake()
{
    status = HandshakeConnecting;
    controlSocket = new QTcpSocket(this);
    controlSocketBuffer.clear();
    connect(controlSocket, SIGNAL(connected()), this, SLOT(controlSocketConnected()));
    connect(controlSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(controlSocketError(QAbstractSocket::SocketError)));
    connect(controlSocket, SIGNAL(readyRead()), this, SLOT(controlSocketRead()));
    controlHandshakeTimer->start(5 * 1000);
    controlSocket->connectToHost(destinationHost, port);
    emit twampLogString("[TCP] Connecting to " + destinationHost.toString() + ":" + QString::number(port));
}

void TwampTestWorker::controlSocketConnected()
{
    controlHandshakeTimer->stop();
    emit twampLogString("[TCP] Connected");
    status = HandshakeServerGreeting;
}

void TwampTestWorker::controlSocketError(QAbstractSocket::SocketError error)
{
    (void) error;
    emit errorMessage(controlSocket->errorString());
    emit testFinished();
}

void TwampTestWorker::controlSocketRead()
{
    controlSocketBuffer.append(controlSocket->readAll());
    if (status == HandshakeServerGreeting) {
        if (controlSocketBuffer.length() < (int) sizeof(struct twamp_message_server_greeting)) {
            emit twampLog(TwampLogPacket, "Received short Greeting, size: " + QString::number(controlSocketBuffer.length()),
                          controlSocketBuffer, HandshakeError);
            return;
        } else {
            emit twampLog(TwampLogPacket, "", controlSocketBuffer, HandshakeServerGreeting);
            sendControlSetupResponse();
        }
    } else if (status == HandshakeSetupResponse) {
        if (controlSocketBuffer.length() < (int) sizeof(struct twamp_message_server_start)) {
            emit twampLog(TwampLogPacket, "Received short Server-Start, size: " + QString::number(controlSocketBuffer.length()),
                          controlSocketBuffer, HandshakeError);
            return;
        } else {
            emit twampLog(TwampLogPacket, "", controlSocketBuffer, HandshakeServerStart);
            struct twamp_message_server_start *start = (struct twamp_message_server_start*)controlSocketBuffer.constData();
            if (start->accept == 0) {
                sendControlRequestSession();
            } else {
                abortControlHandshake();
            }
        }
    } else if (status == HandshakeRequestSession) {
        if (controlSocketBuffer.length() < (int)sizeof(struct twamp_message_accept_session)) {
            emit twampLog(TwampLogPacket, "Received short Accept-Session, size: " + QString::number(controlSocketBuffer.length()),
                          controlSocketBuffer, HandshakeError);
            return;
        } else {
            emit twampLog(TwampLogPacket, "", controlSocketBuffer, HandshakeAcceptSession);
            struct twamp_message_accept_session *accept_session = (struct twamp_message_accept_session*)controlSocketBuffer.constData();
            if (accept_session->accept == 0) {
                receiverUdpPort = qFromBigEndian(accept_session->port);
                sendControlStartSessions();
            } else {
                abortControlHandshake();
            }
        }
    } else if (status == HandshakeStartSession) {
        if (controlSocketBuffer.length() < (int)sizeof(struct twamp_message_start_ack)) {
            emit twampLog(TwampLogPacket, "Receiver short Start-Sessions-Ack, size: " + QString::number(controlSocketBuffer.length()),
                          controlSocketBuffer, HandshakeError);
            return;
        } else {
            emit twampLog(TwampLogPacket, "", controlSocketBuffer, HandshakeStartSessionAck);
            udpSendTimer->start(interval);
        }
    }
    controlSocketBuffer.clear();
}

void TwampTestWorker::abortControlHandshake(QString message)
{
    emit errorMessage(message);
    emit testFinished();
    controlHandshakeFinished();
}

void TwampTestWorker::sendControlSetupResponse()
{
    struct twamp_message_setup_response setup_response;
    quint32 mode = TWAMP_CONTROL_MODE_OPEN;
    memset(&setup_response, 0, sizeof(struct twamp_message_setup_response));
    qToBigEndian(mode, (uchar*)&setup_response.mode);
    controlSocket->write((const char*)&setup_response, sizeof(setup_response));
    status = HandshakeSetupResponse;
    emit twampLog(TwampLogPacket, "", QByteArray((const char*)&setup_response, sizeof(setup_response)), HandshakeSetupResponse);
}

void TwampTestWorker::sendControlRequestSession()
{
    struct twamp_message_request_session request_session;
    memset(&request_session, 0, sizeof(struct twamp_message_request_session));

    quint32 timeout_seconds  = 5;
    quint32 timeout_fraction = 0;
    quint32 dscp = 0;

    twamp_time now = getTwampTime();

    request_session.five = 5;
    request_session.conf_sender = 0;
    request_session.conf_receiver = 0;
    request_session.schedule_slots = 0;
    request_session.packets = 0;
    /* use same port for both sending and receiving */
    qToBigEndian(udpSocket->localPort(), (uchar*)&request_session.sender_port);
    qToBigEndian(udpSocket->localPort(), (uchar*)&request_session.receiver_port);

    if (destinationHost.protocol() == QAbstractSocket::IPv6Protocol) {
        request_session.ipvn = 6;
        Q_IPV6ADDR ip6 = destinationHost.toIPv6Address();
        memcpy(&request_session.receiver_address, &ip6, sizeof(ip6));
    } else {
        request_session.ipvn = 4;
        qToBigEndian(destinationHost.toIPv4Address(), (uchar*)&request_session.receiver_address);
    }

    qToBigEndian(padding_length, (uchar*)&request_session.padding_length);

    qToBigEndian(now.seconds, (uchar*)&request_session.start_time.seconds);
    qToBigEndian(now.fraction, (uchar*)&request_session.start_time.fraction);

    qToBigEndian(timeout_seconds, (uchar*)&request_session.timeout.seconds);
    qToBigEndian(timeout_fraction, (uchar*)&request_session.timeout.fraction);

    qToBigEndian(dscp, (uchar*)&request_session.type_p_descriptor);

    controlSocket->write((const char*)&request_session, sizeof(request_session));
    status = HandshakeRequestSession;
    emit twampLog(TwampLogPacket, "", QByteArray((const char*)&request_session, sizeof(request_session)), HandshakeRequestSession);
}

void TwampTestWorker::sendControlStartSessions()
{
    struct twamp_message_start_sessions start_sessions;
    memset(&start_sessions, 0, sizeof(struct twamp_message_start_sessions));
    start_sessions.two = 2;
    controlSocket->write((const char*)&start_sessions, sizeof(start_sessions));
    status = HandshakeStartSession;
    emit twampLog(TwampLogPacket, "", QByteArray((const char*)&start_sessions, sizeof(start_sessions)), HandshakeStartSession);
}

void TwampTestWorker::sendControlStopSessions()
{
    struct twamp_message_stop_sessions stop_sessions;
    memset(&stop_sessions, 0, sizeof(struct twamp_message_stop_sessions));
    stop_sessions.three = 3;
    controlSocket->write((const char*)&stop_sessions, sizeof(stop_sessions));
    status = HandshakeStopSession;
    emit twampLog(TwampLogPacket, "", QByteArray((const char*)&stop_sessions, sizeof(stop_sessions)), HandshakeStopSession);
}

void TwampTestWorker::doTest()
{
    elapsedTimer->start();
    emit testStarted(totalPackets);
    if (lightMode) {
        receiverUdpPort = (quint16) port;
        udpSendTimer->start(interval);
    } else {
        startControlHandshake();
    }
}

void TwampTestWorker::udpSendTimerDone()
{
    if ((int)sequence >= packets.size()) {
        /* this shouldn't happen */
        return;
    }

    struct udpTestPacket *packet = &packets[sequence];

    struct twamp_time now = getTwampTime();

    packet->sent = elapsedTimer->nsecsElapsed() / 1000;
    packet->index = sequence;
    qToBigEndian(sequence, (uchar*)&message->sequence_number);
    qToBigEndian(now.seconds, (uchar*)&message->timestamp.seconds);
    qToBigEndian(now.fraction, (uchar*)&message->timestamp.fraction);
    message->error_estimate.s = 0;
    message->error_estimate.z = 0;
    message->error_estimate.scale = 0;
    message->error_estimate.multiplier = 1;

    sequence++;

    udpSocket->writeDatagram((const char*)message, message_length, destinationHost, receiverUdpPort);

    emit packetSent(sequence, totalPackets);

    emit twampLog(TwampLogPacket, "", QByteArray((const char*)message, message_length), TestPacketSent);

    if (sequence == totalPackets) {
        udpSendTimer->stop();
        maxTestWaitTimer->start(3 * 1000);
    }
}

void TwampTestWorker::readReflectorMessage()
{
    while (udpSocket->hasPendingDatagrams()) {
        qint64 now = elapsedTimer->nsecsElapsed() / 1000;

        udpSocket->readDatagram((char*)reflector, reflector_length);
        quint32 index = qFromBigEndian(reflector->sequence_number);
        if (packets.size() < (int)index) {
            qDebug() << "invalid sequence no received: " << index;
            return;
        }
        emit twampLog(TwampLogPacket, "", QByteArray((const char*)reflector, reflector_length), TestPacketReceived);

        struct udpTestPacket *packet = &packets[index];
        packet->received = now;
        packet->latency = (now - packet->sent) / 1000.0;
        packet->valid = true;
        received_packets++;
        emit newTestLatency(index, packet->latency);

        if ((index + 1) == totalPackets) {
            /* make chartjs happy again */
            //emit newTestLatency(index + 1, packet->latency / 1000.0);
            processResults();
        }
    }
}

double TwampTestWorker::getStdDev(double average)
{
    double avg = 0;
    double large, small;
    double d_offset, d_square, d_variance = 0;
    struct udpTestPacket *packet;

    for (int i = 0; i < packets.size(); i++) {
        packet = &packets[i];
        if (!packet->valid) {
            continue;
        }

        avg = average;
        if (packet->latency < avg) {
            small = packet->latency;
            large = avg;
        } else {
            large = packet->latency;
            small = avg;
        }
        large -= small;

        d_offset = large;
        d_square = d_offset * d_offset;
        d_variance += d_square;
    }
    return (double) qSqrt(d_variance / totalPackets);
}

void TwampTestWorker::processResults()
{
    double total = 0;
    quint32 success = 0;
    min_latency = 0;
    max_latency = 0;
    average_latency = 0;
    average_jitter = 0;

    struct udpTestPacket *packet;

    maxTestWaitTimer->stop();

    for (int i = 0; i < packets.size(); i++) {
        packet = &packets[i];
        if (!packet->valid) {
            continue;
        }
        success++;
        total += packet->latency;

        if (min_latency == 0 && max_latency == 0) {
            /* first record */
            min_latency = packet->latency;
            max_latency = packet->latency;
        } else if (packet->latency < min_latency) {
            min_latency = packet->latency;
        } else if (packet->latency > max_latency) {
            max_latency = packet->latency;
        }
    }
    if (success > 0) {
        average_latency = total / success;
        average_jitter = getStdDev(average_latency);
    }
    double packetLoss = 100.0f * (totalPackets - success) / totalPackets;
    emit testResultsCalculated(packetLoss, min_latency, max_latency, average_latency, average_jitter);

    if (!lightMode) {
        sendControlStopSessions();
        controlHandshakeFinished();
    }
    emit testFinished();
}

void TwampTestWorker::controlHandshakeTimerDone()
{
    abortControlHandshake("Twamp Control Handshake Timeout");
}

void TwampTestWorker::controlHandshakeFinished()
{
    if (controlSocket) {
        if (controlSocket->isOpen()) {
            controlSocket->close();
        }
        controlSocket->deleteLater();
        controlSocket = NULL;
    }
}

void TwampTestWorker::maxTestWaitTimerDone()
{
    if (received_packets == 0) {
        emit errorMessage("No response received");
    }
    processResults();
}


