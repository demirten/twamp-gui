#ifndef TWAMP_COMMON_H
#define TWAMP_COMMON_H

#include <QObject>
#include <QByteArray>
#include <stdint.h>

#define TWAMP_CONTROL_MODE_UNDEFINED      	(0x00)
#define TWAMP_CONTROL_MODE_OPEN           	(0x01)
#define TWAMP_CONTROL_MODE_AUTHENTICATED  	(0x02)
#define TWAMP_CONTROL_MODE_ENCRYPTED      	(0x04)
#define TWAMP_CONTROL_MODE_DOCIPHER       	(TWAMP_CONTROL_MODE_AUTHENTICATED | TWAMP_CONTROL_MODE_ENCRYPTED)

#define TWAMP_BASE_TIME_OFFSET 2208988800u
#define TWAMP_FLOAT_DENOM 4294.967296

#pragma pack(push,1)
struct twamp_time {
    uint32_t seconds;
    uint32_t fraction;
};

struct twamp_session_sid {
    unsigned char sid[16];
};

struct twamp_session {
    int conf_sender;
    int conf_receiver;
    int sender_port;
    int sender_sock;
    int receiver_port;
    char *sender_address;
    char *receiver_address;
    struct twamp_session_sid sid;
    unsigned int padding_length;
    unsigned int start_time_seconds;
    unsigned int start_time_fraction;
    unsigned int timeout_seconds;
    unsigned int timeout_fraction;
    unsigned int dscp;
    unsigned int number_of_packets;
    unsigned int number_of_slots;
};

struct twamp_message_hwmac {
    uint8_t hwmac[16];
};

struct twamp_message_sid {
    uint8_t sid[16];
};

struct twamp_message_server_greeting {
    uint8_t unused[12];
    uint32_t modes;
    uint8_t challange[16];
    uint8_t salt[16];
    uint32_t count;
    uint8_t mbz[12];
};

struct twamp_message_setup_response {
    uint32_t mode;
    uint8_t keyid[80];
    uint8_t token[64];
    uint8_t client_iv[16];
};

struct twamp_message_server_start {
    uint8_t mbz[15];
    uint8_t accept;
    uint8_t server_iv[16];
    struct twamp_time start_time;
    uint8_t mbz_[8];
};

struct twamp_message_request_session {
    uint8_t five;
    uint8_t ipvn:4;
    uint8_t mbz:4;
    uint8_t conf_sender;		/* 0 */
    uint8_t conf_receiver;		/* 0 */
    uint32_t schedule_slots;	/* 0 */
    uint32_t packets;		/* 0 */
    uint16_t sender_port;
    uint16_t receiver_port;
    uint32_t sender_address[4];
    uint32_t receiver_address[4];
    struct twamp_message_sid sid;	/* 0 */
    uint32_t padding_length;
    struct twamp_time start_time;
    struct twamp_time timeout;
    uint32_t type_p_descriptor;
    uint8_t mbz_[8];
    struct twamp_message_hwmac hwmac;
};

struct twamp_message_accept_session {
    uint8_t accept;
    uint8_t mbz;
    uint16_t port;
    struct twamp_message_sid sid;
    uint8_t mbz_[12];
    struct twamp_message_hwmac hwmac;
};

struct twamp_message_start_sessions {
    uint8_t two;
    uint8_t mbz[15];
    struct twamp_message_hwmac hwmac;
};

struct twamp_message_start_ack {
    uint8_t accept;
    uint8_t mbz[15];
    struct twamp_message_hwmac hwmac;
};

struct twamp_message_stop_sessions {
    uint8_t three;
    uint8_t accept;
    uint8_t mbz[2];
    uint32_t number_of_sessions;
    uint8_t mbz_[8];
    struct twamp_message_hwmac hwmac;
};

struct twamp_message_error_estimate {
    uint8_t s:1;
    uint8_t z:1;
    uint8_t scale:6;
    uint8_t multiplier;
};

struct twamp_message_test_unauthenticated {
    uint32_t sequence_number;
    struct twamp_time timestamp;
    struct twamp_message_error_estimate error_estimate;
};

struct twamp_message_reflector_unathenticated {
    uint32_t sequence_number;
    struct twamp_time timestamp;
    struct twamp_message_error_estimate error_estimate;
    uint8_t mbz[2];
    struct twamp_time receive_timestamp;
    uint32_t sender_sequence_number;
    struct twamp_time sender_timestamp;
    struct twamp_message_error_estimate sender_error_estimate;
    uint8_t mbz_[2];
    uint8_t sender_ttl;
};
#pragma pack(pop)

enum TestStatus {
    StatusUnknown = 0,
    HandshakeConnecting,
    HandshakeServerGreeting,
    HandshakeSetupResponse,
    HandshakeServerStart,
    HandshakeRequestSession,
    HandshakeAcceptSession,
    HandshakeStartSession,
    HandshakeStartSessionAck,
    HandshakeStopSession,
    HandshakeError,
    TestPacketSent,
    TestPacketReceived,
    StatusLast
};

enum TwampLogType {
    TwampLogString,
    TwampLogPacket
};


class TwampCommon : public QObject
{
    Q_OBJECT
public:
    explicit TwampCommon(QObject *parent = 0);

    enum TwampMode { Standard, Light };

    struct twamp_time getTwampTime();
    static QString toHex(const QByteArray &data);
    static QString toHex(const char *data, unsigned int length);
    static QString acceptString(uint8_t accept);

signals:

public slots:

};

#endif // TWAMP_COMMON_H
