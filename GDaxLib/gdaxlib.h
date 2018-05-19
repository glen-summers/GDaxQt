#ifndef GDAXLIB_H
#define GDAXLIB_H

#include "defs.h"
#include "tick.h"
#include "trade.h"

#include "flogging.h"

#include <QObject>
#include <QWebSocket>
QT_FORWARD_DECLARE_CLASS(QJsonObject)

#include <map>
#include <deque>
#include <unordered_map>

class GDaxLib : public QObject
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<GDaxLib>();

    Q_OBJECT

    typedef std::unordered_map<std::string, void(GDaxLib::*)(const QJsonObject & object)> FunctionMap;
    static FunctionMap functionMap;

    QWebSocket webSocket;

    // move to client code?
    std::map<Decimal, Decimal> bids, asks;
    Decimal priceMin;
    Decimal priceMax;
    Decimal amountMax;
    TradeId lastTradeId;

public:
    enum class State
    {
        NotConnected,
        Connecting,
        Connected,
    };

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

    void Ping();

private:
signals:
    void OnUpdate();
    void OnTick(Tick tick);
    void OnStateChanged(State state);

private slots:
    void Connected();
    void TextMessageReceived(QString message);
    void StateChanged(QAbstractSocket::SocketState socketState);
    void Error(QAbstractSocket::SocketError error);
    void SslErrors(const QList<QSslError> &errors);
    void Pong();

private:
    void Clear();
    State ToState(QAbstractSocket::SocketState socketState);
    void ProcessSubscriptions(const QJsonObject & object);
    void ProcessError(const QJsonObject & object);
    void ProcessSnapshot(const QJsonObject & object);
    void ProcessUpdate(const QJsonObject & object);
    void ProcessHeartbeat(const QJsonObject & object);
    void ProcessTicker(const QJsonObject & object);
};

#endif // GDAXLIB_H
