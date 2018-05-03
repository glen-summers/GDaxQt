#ifndef GDAXLIB_H
#define GDAXLIB_H

// discover/relative path/ or use unofficial QDecimal
#define DEC_NAMESPACE DecNs
#include "decimal.h"

#include <QObject>
#include <QWebSocket>
QT_FORWARD_DECLARE_CLASS(QJsonObject)

#include <map>

class GDaxLib : public QObject
{
    typedef DecNs::decimal<10> Decimal;

    Q_OBJECT

    QWebSocket webSocket;

    std::map<Decimal, Decimal> bids, asks;
    Decimal priceMin;
    Decimal priceMax;
    Decimal amountMax;

signals:
    void update();

public:
    explicit GDaxLib(QObject * parent = nullptr);

    const std::map<Decimal, Decimal> & Bids() const
    {
        return bids;
    }

    const std::map<Decimal, Decimal> & Asks() const
    {
        return asks;
    }

    const Decimal & PriceMin() const
    {
        return priceMin;
    }

    const Decimal & PriceMax() const
    {
        return priceMax;
    }

    const Decimal & AmountMax() const
    {
        return amountMax;
    }

private slots:
    void onConnected();

    void onTextMessageReceived(QString message);

    void onError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);

    void ProcessSnapshot(const QJsonObject & object);
    void ProcessUpdate(const QJsonObject & object);
};

#endif // GDAXLIB_H
