#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QString>
#include <QByteArray>

#include <memory>

namespace GDL
{
    struct Auth;
}

class Authenticator
{
    QByteArray apiKey, secretKey, passphrase;

public:
    static std::unique_ptr<Authenticator> Create(GDL::Auth * auth);

    Authenticator(QByteArray apiKey, QByteArray secretKey, QByteArray passphrase);

    const QByteArray & ApiKey() const;
    const QByteArray & Passphrase() const;
    QByteArray ComputeSignature(const QString & httpMethod, QString timestamp, const QString & requestPath, const QByteArray * contentBody);
};

#endif // AUTHENTICATOR_H
