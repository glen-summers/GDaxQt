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

QByteArray Authenticator::ComputeSignature(const QString & httpMethod, QString timestamp, const QString & requestPath,
                                           const QByteArray & contentBody)
{
    QByteArray what;
    what.reserve(timestamp.length() + httpMethod.length() + requestPath.length() + contentBody.length());
    what.append(timestamp).append(httpMethod).append(requestPath).append(contentBody);
    return QMessageAuthenticationCode::hash(what, secretKey, QCryptographicHash::Algorithm::Sha256).toBase64();
}
