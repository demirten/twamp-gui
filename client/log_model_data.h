#ifndef LOG_MODEL_DATA_H
#define LOG_MODEL_DATA_H

#include <QObject>
#include <QByteArray>
#include <QtEndian>
#include <QStringList>
#include <QDateTime>
#include <QHostAddress>
#include "twamp_common.h"

class LogModelData : public QObject
{
    Q_OBJECT
public:
    LogModelData(int type,
                 QString message,
                 QByteArray data,
                 int status,
                 double time)
    {
        mMessage = message;
        mData = data;
        mType = (enum TwampLogType) type;
        mStatus = (enum TestStatus) status;
        mTime = time;
    }

    Q_PROPERTY(QString summary READ summary NOTIFY summaryChanged)
    Q_PROPERTY(QString timing READ timing NOTIFY timingChanged)

    QString summary() {
        QString str = "";
        if (mType == TwampLogString) {
            str = mMessage;
        } else if (mStatus == TestPacketSent) {
            struct twamp_message_test_unauthenticated *msg = (struct twamp_message_test_unauthenticated *)mData.constData();
            str += "[TEST] Packet Sent :: Sequence " + QString::number(qFromBigEndian(msg->sequence_number));
        } else if (mStatus == TestPacketReceived) {
            struct twamp_message_reflector_unathenticated *msg = (struct twamp_message_reflector_unathenticated *)mData.constData();
            str += "[TEST] Packet Received :: Sequence " + QString::number(qFromBigEndian(msg->sequence_number));;
        } else if (mStatus == HandshakeServerGreeting) {
            str += "[CONTROL] Received Server-Greeting";
        } else if (mStatus == HandshakeSetupResponse) {
            str += "[CONTROL] Sent Setup-Response";
        } else if (mStatus == HandshakeServerStart) {
            str += "[CONTROL] Received Server-Start";
        } else if (mStatus == HandshakeRequestSession) {
            str += "[CONTROL] Sent Request-Session";
        } else if (mStatus == HandshakeAcceptSession) {
            str += "[CONTROL] Received Accept-Session";
        } else if (mStatus == HandshakeStartSession) {
            str += "[CONTROL] Sent Start-Sessions";
        } else if (mStatus == HandshakeStartSessionAck) {
            str += "[CONTROL] Received Start-Sessions-ACK";
        } else if (mStatus == HandshakeStopSession) {
            str += "[CONTROL] Sent Stop-Sessions";
        } else if (mStatus == HandshakeError) {
            str += "[CONTROL] Handshake Error: " + mMessage;
        }
        return str;
    }
    QString timing() {
        return QString::number(mTime, 'f', 03);
    }
    Q_INVOKABLE QStringList detail() {
        QStringList items;
        if (mType == TwampLogString) {
            return items;
        }

        /* initialize variables for switch block */
        struct twamp_message_server_greeting *greeting;
        struct twamp_message_setup_response *setup_response;
        struct twamp_message_server_start *server_start;
        struct twamp_message_request_session *request_session;
        struct twamp_message_accept_session *accept_session;
        struct twamp_message_start_sessions *start_sessions;
        struct twamp_message_start_ack *start_ack;
        struct twamp_message_stop_sessions *stop_sessions;
        struct twamp_message_test_unauthenticated *msg_test_unauthenticated;
        struct twamp_message_reflector_unathenticated *msg_reflector_unauthenticated;
        quint32 modes;
        quint32 mode;
        qint64 seconds;
        double msecs;
        QDateTime timestamp;
        QStringList tmpList;
        QHostAddress senderAddr;
        QHostAddress receiverAddr;

        switch (mStatus) {
        case HandshakeServerGreeting:
            greeting = (struct twamp_message_server_greeting *)mData.constData();

            items.append("Unused");
            items.append(TwampCommon::toHex((const char*)greeting->unused, sizeof(greeting->unused)));

            modes = qFromBigEndian(greeting->modes);
            items.append("Supported Modes");
            tmpList.clear();
            if (modes == TWAMP_CONTROL_MODE_UNDEFINED) {
                tmpList.append("UNDEFINED - Server doesn't want to talk with us");
            }
            if (modes & TWAMP_CONTROL_MODE_OPEN) {
                tmpList.append("Unauthenticated");
            }
            if (modes & TWAMP_CONTROL_MODE_AUTHENTICATED) {
                tmpList.append("Authenticated");
            }
            if (modes & TWAMP_CONTROL_MODE_ENCRYPTED) {
                tmpList.append("Encrypted");
            }
            items.append(QString::number(modes) + " (" + tmpList.join(" | ") + ")");

            items.append("Challenge");
            items.append(TwampCommon::toHex((const char*)greeting->challange, sizeof(greeting->challange)));

            items.append("Salt");
            items.append(TwampCommon::toHex((const char*)greeting->salt, sizeof(greeting->salt)));

            items.append("Count");
            items.append(QString::number(qFromBigEndian(greeting->count)));

            items.append("MBZ");
            items.append(TwampCommon::toHex((const char*)greeting->mbz, sizeof(greeting->mbz)));
            break;
        case HandshakeSetupResponse:
            setup_response = (struct twamp_message_setup_response*)mData.constData();
            mode = qFromBigEndian(setup_response->mode);
            items.append("Mode");
            items.append(QString::number(mode));

            items.append("Key ID");
            items.append(TwampCommon::toHex((const char*)setup_response->keyid, sizeof(setup_response->keyid)));

            items.append("Token");
            items.append(TwampCommon::toHex((const char*)setup_response->token, sizeof(setup_response->token)));

            items.append("Client IV");
            items.append(TwampCommon::toHex((const char*)setup_response->client_iv, sizeof(setup_response->client_iv)));
            break;
        case HandshakeServerStart:
            server_start = (struct twamp_message_server_start*)mData.constData();
            items.append("MBZ1");
            items.append(TwampCommon::toHex((const char*)server_start->mbz, sizeof(server_start->mbz)));

            items.append("Accept");
            items.append(QString::number(server_start->accept) + " (" + TwampCommon::acceptString(server_start->accept) + ")");

            items.append("Server IV");
            items.append(TwampCommon::toHex((const char*)server_start->server_iv, sizeof(server_start->server_iv)));

            seconds = qFromBigEndian(server_start->start_time.seconds) - TWAMP_BASE_TIME_OFFSET;
            msecs   = qFromBigEndian(server_start->start_time.fraction) / TWAMP_FLOAT_DENOM;
            timestamp = QDateTime::fromMSecsSinceEpoch(seconds * 1000 + msecs);
            items.append("Server Start Time");
            items.append(timestamp.toString("dd.MM.yyyy hh:mm:ss.zzz"));
            if (seconds > ((QDateTime::currentMSecsSinceEpoch() / 1000) + (5 * 365 * 86400))) {
                /* 5 year threshold enough to check violation */
                items.append("Warning");
                items.append("Server not using NTP Timestamps, violetes RFC");
            }

            items.append("MBZ2");
            items.append(TwampCommon::toHex((const char*)server_start->mbz_, sizeof(server_start->mbz_)));
            break;
        case HandshakeRequestSession:
            request_session = (struct twamp_message_request_session*)mData.constData();
            items.append("Five");
            items.append(QString::number(request_session->five));

            items.append("IP Version");
            items.append(QString::number(request_session->ipvn));

            items.append("Conf Sender");
            items.append(QString::number(request_session->conf_sender));

            items.append("Conf Receiver");
            items.append(QString::number(request_session->conf_receiver));

            items.append("Number of Slots");
            items.append(QString::number(qFromBigEndian(request_session->schedule_slots)));

            items.append("Number of Packets");
            items.append(QString::number(qFromBigEndian(request_session->packets)) + " (must be zero on TWAMP)");

            items.append("Sender Port");
            items.append(QString::number(qFromBigEndian(request_session->sender_port)));

            items.append("Receiver Port");
            items.append(QString::number(qFromBigEndian(request_session->receiver_port)));

            if (request_session->ipvn == 6) {
                senderAddr.setAddress((quint8*)&request_session->sender_address);
                receiverAddr.setAddress((quint8*)&request_session->receiver_address);
            } else {
                senderAddr.setAddress(qFromBigEndian((quint32)request_session->sender_address[0]));
                receiverAddr.setAddress(qFromBigEndian((quint32)request_session->receiver_address[0]));
            }
            items.append("Sender Address");
            items.append(senderAddr.toString());
            items.append("Receiver Address");
            items.append(receiverAddr.toString());

            items.append("Session ID");
            items.append(TwampCommon::toHex((const char*)&request_session->sid, sizeof(request_session->sid)));

            items.append("Padding Length");
            items.append(QString::number(qFromBigEndian(request_session->padding_length)));

            seconds = qFromBigEndian(request_session->start_time.seconds) - TWAMP_BASE_TIME_OFFSET;
            msecs   = qFromBigEndian(request_session->start_time.fraction) / TWAMP_FLOAT_DENOM;
            timestamp = QDateTime::fromMSecsSinceEpoch(seconds * 1000 + msecs);
            items.append("Start Time");
            items.append(timestamp.toString("dd.MM.yyyy hh:mm:ss.zzz"));

            seconds = qFromBigEndian(request_session->timeout.seconds);
            msecs   = qFromBigEndian(request_session->timeout.fraction);
            items.append("Timeout");
            items.append(QString::number(seconds) + "." + QString::number(msecs) + " seconds");

            items.append("Type-P Descriptor");
            items.append(QString::number(qFromBigEndian(request_session->type_p_descriptor)));

            items.append("MBZ");
            items.append(TwampCommon::toHex((const char*)&request_session->mbz_, sizeof(request_session->mbz_)));

            items.append("HWMAC");
            items.append(TwampCommon::toHex((const char*)&request_session->hwmac, sizeof(request_session->hwmac)));

            break;
        case HandshakeAcceptSession:
            accept_session = (struct twamp_message_accept_session *)mData.constData();

            items.append("Accept");
            items.append(QString::number(accept_session->accept) + " (" + TwampCommon::acceptString(accept_session->accept) + ")");

            items.append("MBZ1");
            items.append(TwampCommon::toHex((const char *)&accept_session->mbz, sizeof(accept_session->mbz)));

            items.append("Receiver Port");
            items.append(QString::number(qFromBigEndian(accept_session->port)));

            items.append("Session ID");
            items.append(TwampCommon::toHex((const char*)&accept_session->sid, sizeof(accept_session->sid)));

            items.append("MBZ2");
            items.append(TwampCommon::toHex((const char*)&accept_session->mbz_, sizeof(accept_session->mbz_)));

            items.append("HWMAC");
            items.append(TwampCommon::toHex((const char*)&accept_session->hwmac, sizeof(accept_session->hwmac)));
            break;
        case HandshakeStartSession:
            start_sessions = (struct twamp_message_start_sessions*)mData.constData();
            items.append("Two");
            items.append(QString::number(start_sessions->two));

            items.append("MBZ");
            items.append(TwampCommon::toHex((const char*)&start_sessions->mbz, sizeof(start_sessions->mbz)));

            items.append("HWMAC");
            items.append(TwampCommon::toHex((const char*)&start_sessions->hwmac, sizeof(start_sessions->hwmac)));
            break;
        case HandshakeStartSessionAck:
            start_ack = (struct twamp_message_start_ack*)mData.constData();
            items.append("Accept");
            items.append(QString::number(start_ack->accept) + " (" + TwampCommon::acceptString(start_ack->accept) + ")");

            items.append("MBZ");
            items.append(TwampCommon::toHex((const char *)&start_ack->mbz, sizeof(start_ack->mbz)));

            items.append("HWMAC");
            items.append(TwampCommon::toHex((const char*)&start_ack->hwmac, sizeof(start_ack->hwmac)));
            break;
        case HandshakeStopSession:
            stop_sessions = (struct twamp_message_stop_sessions*)mData.constData();
            items.append("Three");
            items.append(QString::number(stop_sessions->three));

            items.append("Accept");
            items.append(QString::number(stop_sessions->accept) + " (" + TwampCommon::acceptString(stop_sessions->accept) + ")");

            items.append("MBZ1");
            items.append(TwampCommon::toHex((const char *)&stop_sessions->mbz, sizeof(stop_sessions->mbz)));

            items.append("Number of Sessions");
            items.append(QString::number(qFromBigEndian(stop_sessions->number_of_sessions)));

            items.append("MBZ2");
            items.append(TwampCommon::toHex((const char *)&stop_sessions->mbz_, sizeof(stop_sessions->mbz_)));

            items.append("HWMAC");
            items.append(TwampCommon::toHex((const char*)&stop_sessions->hwmac, sizeof(stop_sessions->hwmac)));

            break;
        case TestPacketSent:
            msg_test_unauthenticated = (struct twamp_message_test_unauthenticated *)mData.constData();
            items.append("Sequence");
            items.append(QString::number(qFromBigEndian(msg_test_unauthenticated->sequence_number)));
            seconds = qFromBigEndian(msg_test_unauthenticated->timestamp.seconds) - TWAMP_BASE_TIME_OFFSET;
            msecs   = qFromBigEndian(msg_test_unauthenticated->timestamp.fraction) / TWAMP_FLOAT_DENOM;
            timestamp = QDateTime::fromMSecsSinceEpoch(seconds * 1000 + msecs);
            items.append("Sent");
            items.append(timestamp.toString("dd.MM.yyyy hh:mm:ss.zzz"));
            break;
        case TestPacketReceived:
            msg_reflector_unauthenticated = (struct twamp_message_reflector_unathenticated*)mData.constData();
            items.append("Sender Sequence");
            items.append(QString::number(qFromBigEndian(msg_reflector_unauthenticated->sequence_number)));
            items.append("Sender TTL");
            items.append(QString::number(msg_reflector_unauthenticated->sender_ttl));
            seconds = qFromBigEndian(msg_reflector_unauthenticated->receive_timestamp.seconds) - TWAMP_BASE_TIME_OFFSET;
            msecs   = qFromBigEndian(msg_reflector_unauthenticated->receive_timestamp.fraction) / TWAMP_FLOAT_DENOM;
            timestamp = QDateTime::fromMSecsSinceEpoch(seconds * 1000 + msecs);
            items.append("Receive Timestamp");
            items.append(timestamp.toString("dd.MM.yyyy hh:mm:ss.zzz"));
            seconds = qFromBigEndian(msg_reflector_unauthenticated->timestamp.seconds) - TWAMP_BASE_TIME_OFFSET;
            msecs   = qFromBigEndian(msg_reflector_unauthenticated->timestamp.fraction) / TWAMP_FLOAT_DENOM;
            timestamp = QDateTime::fromMSecsSinceEpoch(seconds * 1000 + msecs);
            items.append("Sent Timestamp");
            items.append(timestamp.toString("dd.MM.yyyy hh:mm:ss.zzz"));
            items.append("Reflector Sequence");
            items.append(QString::number(qFromBigEndian(msg_reflector_unauthenticated->sequence_number)));
            break;
        default:
            break;
        }
        return items;
    }

private:
    QString mMessage;
    QByteArray mData;
    enum TwampLogType mType;
    enum TestStatus mStatus;
    double mTime;
signals:
    void summaryChanged();
    void timingChanged();
};

#endif // LOG_MODEL_DATA_H
