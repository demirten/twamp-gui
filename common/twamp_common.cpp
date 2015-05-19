#include "twamp_common.h"
#include <QDateTime>

TwampCommon::TwampCommon(QObject *parent) :
    QObject(parent)
{
}

struct twamp_time TwampCommon::getTwampTime()
{
    struct twamp_time t;
    t.seconds = QDateTime::currentMSecsSinceEpoch() / 1000 + TWAMP_BASE_TIME_OFFSET;
    t.fraction = (QDateTime::currentMSecsSinceEpoch() % 1000) * TWAMP_FLOAT_DENOM;
    return t;
}

QString TwampCommon::toHex(const QByteArray &data)
{
    return toHex(data.constData(), data.size());
}

QString TwampCommon::toHex(const char *data, unsigned int length)
{
    QString resHex = "";
    for (unsigned int i = 0; i < length; i++) {
        resHex.append(QString::number((unsigned char)data[i], 16).rightJustified(2, '0'));
    }
    return resHex;
}

QString TwampCommon::acceptString(uint8_t accept)
{
    switch (accept) {
        case 0: return "OK";
        case 1: return "Failure, reason unspecified (catch-all).";
        case 2: return "Internal error.";
        case 3: return "Some aspect of request is not supported.";
        case 4: return "Cannot perform request due to permanent resource limitations.";
        case 5: return "Cannot perform request due to temporary resource limitations.";
    }
    return "Unknown accept value";
}
