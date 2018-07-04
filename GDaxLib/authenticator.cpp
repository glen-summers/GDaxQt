#include "authenticator.h"
#include "gdl.h"

#include <QDebug>

#include <QString>
#include <QStringBuilder>
#include <QMessageAuthenticationCode>

std::unique_ptr<Authenticator> Authenticator::Create(GDL::Auth * auth)
{
    return auth ? std::make_unique<Authenticator>(auth->key, QByteArray::fromBase64(auth->secret), auth->passphrase)
                : nullptr;
}

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
                                           const QByteArray * contentBody)
{
    int length = timestamp.length() + httpMethod.length() + requestPath.length();
    if (contentBody)
    {
        length += contentBody->length();
    }

    QByteArray what;
    what.reserve(length);
    what.append(timestamp).append(httpMethod).append(requestPath);
    if (contentBody)
    {
        what.append(*contentBody);
    }
    return QMessageAuthenticationCode::hash(what, secretKey, QCryptographicHash::Algorithm::Sha256).toBase64();
}
