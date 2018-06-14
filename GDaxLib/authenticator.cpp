#include "authenticator.h"

#include <QDebug>

#include <QString>
#include <QStringBuilder>
#include <QMessageAuthenticationCode>

Authenticator::Authenticator(QByteArray apiKey, QByteArray secretKey, QByteArray passphrase)
 : apiKey(std::move(apiKey))
 , secretKey(std::move(secretKey))
 , passphrase(std::move(passphrase))
{
}

const QByteArray &Authenticator::ApiKey() const
{
    return apiKey;
}

const QByteArray &Authenticator::Passphrase() const
{
    return passphrase;
}

QByteArray Authenticator::ComputeSignature(const QString & httpMethod, time_t timestamp, const QString & requestPath,
                                           const QString & contentBody)
{
    // try QMessageAuthenticationCode incremental and contentBody as bytearray
    QString what = QString::number(timestamp) % httpMethod % requestPath % contentBody;
    return QMessageAuthenticationCode::hash(what.toUtf8(), secretKey, QCryptographicHash::Algorithm::Sha256).toBase64();
}
