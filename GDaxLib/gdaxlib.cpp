#include "gdaxlib.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QMetaEnum>

namespace
{
// cfg. allow using sandbox
    static constexpr const char * url = "wss://ws-feed.gdax.com";

    static constexpr const char * subscribeMessage = R"(
{
"type": "subscribe",
"product_ids": [
    "BTC-EUR"
],
"channels": [
    "level2",
    "heartbeat",
    "ticker"
]
})";

}

GDaxLib::FunctionMap GDaxLib::functionMap =
{
    { "snapshot", &GDaxLib::ProcessSnapshot },
    { "l2update", &GDaxLib::ProcessUpdate },
    { "heartbeat", &GDaxLib::ProcessHeartbeat },
    { "ticker", &GDaxLib::ProcessTicker},
    { "error", &GDaxLib::ProcessError }
};

GDaxLib::GDaxLib(QObject * parent) // parent?
    : QObject(parent)
    , lastTradeId()
{
    // proxy?
    // QList<QNetworkProxy> QNetworkProxyFactory::systemProxyForQuery(const QNetworkProxyQuery &query = QNetworkProxyQuery())

    connect(&webSocket, &QWebSocket::connected, this, &GDaxLib::Connected);
    connect(&webSocket, &QWebSocket::textMessageReceived, this, &GDaxLib::TextMessageReceived);
    connect(&webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &GDaxLib::Error);
    connect(&webSocket, QOverload<const QList<QSslError> &>::of(&QWebSocket::sslErrors), this, &GDaxLib::SslErrors);

    log.Info(QString("Connecting to %1").arg(url));
    webSocket.open(QUrl(url));
}

void GDaxLib::Connected()
{
    // need qInstallMessageHandler(SyslogMessageHandler);? and handle ourselves?
    // https://stackoverflow.com/questions/28540571/how-to-enable-and-disable-qdebug-messages
    // https://gist.github.com/polovik/10714049
    log.Info("onConnected, subscribing...");

    webSocket.sendTextMessage(subscribeMessage);
}

void GDaxLib::TextMessageReceived(QString message)
{
    try
    {
        QJsonDocument document = QJsonDocument::fromJson(message.toUtf8());
        QJsonObject object = document.object();
        QString type = object["type"].toString();

        auto it = functionMap.find(type.toStdString());
        if (it!=functionMap.end())
        {
            //std::invoke(it->second, *this, object); // try again with correct flags?
            (this->*(it->second))(object);
        }
        else
        {
            log.Warning(QString("Unprocessed message: %1").arg(type));
        }
    }
    catch (const std::exception & e)
    {
        log.Error(QString("Error: %1 , %2").arg(e.what()).arg(message));
    }
}

void GDaxLib::Error(QAbstractSocket::SocketError error)
{
    log.Error(QString("SocketError: %1").arg(QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(error)));
}

void GDaxLib::SslErrors(const QList<QSslError> &errors)
{
    for(auto & e : errors)
    {
        log.Error(QString("SslError: %1, %2").arg(e.error()).arg(e.errorString()));
    }
}

void GDaxLib::ProcessError(const QJsonObject &object)
{
    // will be in the message pump, so cannot throw? need to notify ui
    QString errorMessage = object["message"].toString();
    log.Error(QString("Error message: %1").arg(errorMessage));
    throw std::runtime_error(errorMessage.toStdString());
}

void GDaxLib::ProcessSnapshot(const QJsonObject & object)
{
    log.Info("Snapshot");

    Decimal totBid;
    for (QJsonValueRef bid : object["bids"].toArray())
    {
        QJsonArray bidArray = bid.toArray();
        QString price = bidArray[0].toString();
        QString amount = bidArray[1].toString();

        Decimal dp(price.toStdString());
        Decimal da(amount.toStdString());
        bids[dp] += da;
        totBid += da;
    }

    Decimal totAsk;
    for (QJsonValueRef ask : object["asks"].toArray())
    {
        QJsonArray askArray = ask.toArray();
        QString price = askArray[0].toString();
        QString amount = askArray[1].toString();

        Decimal dp(price.toStdString());
        Decimal da(amount.toStdString());
        asks[dp] += da;
        //amountMax = std::max(amountMax, da);
        totAsk += da;
    }

    Decimal target = (totBid + totAsk) / 1000; // .1% -- move to depths chart
    Decimal tot;

    auto bidIt = bids.rbegin();
    auto askIt = asks.begin();
    for(; tot<target;++bidIt,++askIt)
    {
        if (bidIt!=bids.rend())
        {
            tot += bidIt->second;
        }
        if (askIt!=asks.end())
        {
            tot += askIt->second;
        }
    }
    // check for end
    priceMin = bidIt->first;
    priceMax = askIt->first;
    amountMax = tot;

    emit OnUpdate();
}

void GDaxLib::ProcessUpdate(const QJsonObject & object)
{
    log.Info("Update");
    for(QJsonValueRef changes : object["changes"].toArray())
    {
        QJsonArray array = changes.toArray();
        QString  changeType = array[0].toString();
        bool bid = changeType == "buy";

        Decimal pd(array[1].toString().toStdString());
        Decimal ad(array[2].toString().toStdString());
        bool zeroSize = ad == Decimal();

        if (bid)
        {
            if (zeroSize)
            {
                bids.erase(pd);
            }
            else
            {
                bids[pd] = ad;
            }
        }
        else
        {
            if (zeroSize)
            {
                asks.erase(pd);
            }
            else
            {
                asks[pd] = ad;
            }
        }
    }
    // adjust scales here?

    emit OnUpdate();
}

void GDaxLib::ProcessHeartbeat(const QJsonObject & object)
{
    auto seq = static_cast<SequenceNumber>(object["sequence"].toDouble());
    auto tradeId = static_cast<TradeId>(object["last_trade_id"].toDouble());
    QString serverTime = object["time"].toString();

    if (lastTradeId!=0 && lastTradeId+1 != tradeId)
    {
        log.Info(QString("[%1] [%2] MissedTrade(s): %3 - %4")
                 .arg(serverTime)
                 .arg(seq)
                 .arg(lastTradeId+1)
                 .arg(tradeId-1)
                 .arg(serverTime));
    }
}

void GDaxLib::ProcessTicker(const QJsonObject & object)
{
    log.Info("Tick");

    auto tick(Tick::FromJson(object));
    if (tick.side == TakerSide::None)
    {
        return;
    }

    emit OnTick(tick);
}
