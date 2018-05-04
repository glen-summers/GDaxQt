#ifndef GDAXLIB_H
#define GDAXLIB_H

#include "defs.h"
#include "tick.h"

#include <QObject>
#include <QWebSocket>
QT_FORWARD_DECLARE_CLASS(QJsonObject)

#include <map>
#include <deque>
#include <unordered_map>

class GDaxLib : public QObject
{
    Q_OBJECT

    typedef std::deque<Tick> TicksType;
    typedef std::unordered_map<std::string, void(GDaxLib::*)(const QJsonObject & object)> FunctionMap;
    static FunctionMap functionMap;


    QWebSocket webSocket;

    std::map<Decimal, Decimal> bids, asks;
    Decimal priceMin;
    Decimal priceMax;
    Decimal amountMax;
    TicksType ticks;

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

    const TicksType & Ticks() const
    {
        return ticks;
    }

private slots:
    void onConnected();

    void onTextMessageReceived(QString message);

    void onError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);

    void ProcessError(const QJsonObject & object);
    void ProcessSnapshot(const QJsonObject & object);
    void ProcessUpdate(const QJsonObject & object);
    void ProcessHeartbeat(const QJsonObject & object);
    void ProcessTicker(const QJsonObject & object);
};

#endif // GDAXLIB_H
