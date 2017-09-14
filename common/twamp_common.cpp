#include "twamp_common.h"
#include <QDateTime>
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include <sys/timex.h>
#endif

TwampCommon::TwampCommon(QObject *parent) :
    QObject(parent)
{
}

struct twamp_time TwampCommon::getTwampTime()
{
    struct twamp_time t;
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    t.seconds = (currentTime / 1000) + TWAMP_BASE_TIME_OFFSET;
    /* convert milliseconds to microseconds multiplying by 1000 */
    t.fraction = (currentTime % 1000) * TWAMP_FLOAT_DENOM * 1000;
    return t;
}

int TwampCommon::twampTimeToTimeval (struct twamp_time *time, struct timeval *result)
{
    result->tv_sec  = (time_t)(time->seconds - TWAMP_BASE_TIME_OFFSET);
    result->tv_usec = (time_t)(time->fraction / TWAMP_FLOAT_DENOM);
    return 0;
}

float TwampCommon::timevalDiff (struct timeval *before, struct timeval *after)
{
    time_t usecs = after->tv_usec - before->tv_usec;
    time_t secs = after->tv_sec - before->tv_sec;

    if (usecs < 0) {
        usecs += 1000000;
        secs--;
    }
    return (secs * 1000 + (usecs / 1000.0));
}

struct twamp_message_error_estimate TwampCommon::getErrorEstimate()
{
    struct twamp_message_error_estimate estimate;
    memset(&estimate, 0, sizeof(estimate));

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    struct timex ntp_conf;
    memset(&ntp_conf, 0, sizeof(ntp_conf));

    if ((ntp_adjtime(&ntp_conf) != -1) && !(ntp_conf.status & STA_UNSYNC)) {
        /* NTP sync is active */
        estimate.s = 1;

        /* Convert error estimate from microseconds
           to Multiplier*2^(-32)*2^Scale (in seconds) */
        qint64 error = (ntp_conf.esterror << 32) / 1000000;

        /* Shift error until it fits into 8 bits */
        while (error >= 0xFF) {
            estimate.scale++;
            error >>= 1;
        }

        /* Add one for rounding error */
        estimate.multiplier = error + 1;
    }
#endif

    return estimate;
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
