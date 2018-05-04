#include "gdaxlib.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>

namespace
{
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
{
    connect(&webSocket, &QWebSocket::connected, this, &GDaxLib::onConnected);

    typedef void (QWebSocket:: *sslErrorsSignal)(const QList<QSslError> &);

    connect(&webSocket, &QWebSocket::textMessageReceived, this, &GDaxLib::onTextMessageReceived);
    connect(&webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &GDaxLib::onError);
    connect(&webSocket, static_cast<sslErrorsSignal>(&QWebSocket::sslErrors), this, &GDaxLib::onSslErrors);

    webSocket.open(QUrl(url));
}

void GDaxLib::onConnected()
{
    // need qInstallMessageHandler(SyslogMessageHandler);? and handle ourselves?
    // https://stackoverflow.com/questions/28540571/how-to-enable-and-disable-qdebug-messages
    // https://gist.github.com/polovik/10714049
    qInfo("onConnected");

    webSocket.sendTextMessage(subscribeMessage);
}

void GDaxLib::onTextMessageReceived(QString message)
{
    try
    {
        QJsonDocument document = QJsonDocument::fromJson(message.toUtf8());
        QJsonObject object = document.object();
        QString type = object["type"].toString();

        auto it = functionMap.find(type.toUtf8().constData());
        if (it!=functionMap.end())
        {
            std::invoke(it->second, *this, object);
        }
        else
        {
            qWarning((std::string("Unprocessed message : ") +
                   std::to_string((long long)QThread::currentThreadId()) + " : " +
                   message.toUtf8().constData()).c_str());
        }
    }
    catch (const std::exception & e)
    {
        qWarning((std::string("Error : ") +
               std::to_string((long long)QThread::currentThreadId()) + " : " +
               e.what()).c_str());
    }
}

void GDaxLib::onError(QAbstractSocket::SocketError error)
{
    qWarning(std::to_string((int)error).c_str());
}

void GDaxLib::onSslErrors(const QList<QSslError> &errors)
{
    for(auto & e : errors)
    {
        qWarning(e.errorString().toUtf8());
    }
}

void GDaxLib::ProcessError(const QJsonObject &object)
{
    QString errorMessage = object["message"].toString();
    throw std::runtime_error(errorMessage.toUtf8().constData());
}

void GDaxLib::ProcessSnapshot(const QJsonObject & object)
{
    Decimal totBid;
    for (QJsonValueRef bid : object["bids"].toArray())
    {
        QJsonArray bidArray = bid.toArray();
        QString price = bidArray[0].toString();
        QString amount = bidArray[1].toString();

        Decimal dp(price.toUtf8().constData());
        Decimal da(amount.toUtf8().constData());
        bids[dp] += da;
        totBid += da;
    }

    Decimal totAsk;
    for (QJsonValueRef ask : object["asks"].toArray())
    {
        QJsonArray askArray = ask.toArray();
        QString price = askArray[0].toString();
        QString amount = askArray[1].toString();

        Decimal dp(price.toUtf8().constData());
        Decimal da(amount.toUtf8().constData());
        asks[dp] += da;
        //amountMax = std::max(amountMax, da);
        totAsk += da;
    }

    Decimal target = (totBid + totAsk) / 1000; // .1%
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

    emit update();
}

void GDaxLib::ProcessUpdate(const QJsonObject & object)
{
    for(QJsonValueRef changes : object["changes"].toArray())
    {
        QJsonArray array = changes.toArray();
        QString  changeType = array[0].toString();
        bool bid = changeType == "buy";

        Decimal pd(array[1].toString().toUtf8().constData());
        Decimal ad(array[2].toString().toUtf8().constData());
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

    emit update();
}

void GDaxLib::ProcessHeartbeat(const QJsonObject & object)
{
    auto s = object["sequence"];
    auto seq = static_cast<unsigned long long>(s.toVariant().toDouble());
    auto tradeId = static_cast<unsigned long long>(object["last_trade_id"].toDouble());
    QString time = object["time"].toString();
    //qInfo(QString("HB: %1 %2 %3").arg(seq).arg(tradeId).arg(time).toUtf8().constData());
}

void GDaxLib::ProcessTicker(const QJsonObject & object)
{
    auto s = object["sequence"];
    auto seq = static_cast<unsigned long long>(s.toVariant().toDouble());
    auto tradeId = static_cast<unsigned long long>(object["last_trade_id"].toDouble());
    QString time = object["time"].toString();
    QString side = object["side"].toString();
    QString price = object["price"].toString();
    QString size = object["last_size"].toString();

    qInfo(QString("Ticker: %1 %2 %3 %4 %5 %6").arg(seq).arg(tradeId).arg(time, side, price, size).toUtf8().constData());
}
