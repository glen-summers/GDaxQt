#include "gdaxlib.h"

#include "tick.h"
#include "utils.h"

#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QMetaEnum>
#include <QNetworkReply>
#include <QTimer>

namespace
{
    static constexpr int PingTimerMs = 5000;

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

    inline static const Flog::Log flog = Flog::LogManager::GetLog<GDaxLib>(); // log ambiguous

    // informational atm
    void Error(QAbstractSocket::SocketError error)
    {
        flog.Error(QString("SocketError: %1").arg(QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error)));
    }

    void SslErrors(QList<QSslError> errors)
    {
        for(auto & e : errors)
        {
            flog.Error(QString("SslError: %1, %2").arg(e.error()).arg(e.errorString()));
        }
    }

    ConnectedState ToState(QAbstractSocket::SocketState socketState)
    {
        switch (socketState)
        {
            case QAbstractSocket::ConnectingState:
                return ConnectedState::Connecting;

            case QAbstractSocket::ConnectedState:
                return ConnectedState::Connected;

            default:
                return ConnectedState::NotConnected;;
        };
    }
}

GDaxLib::FunctionMap GDaxLib::functionMap =
{
    { "subscriptions", &GDaxLib::ProcessSubscriptions },
    { "snapshot", &GDaxLib::ProcessSnapshot },
    { "l2update", &GDaxLib::ProcessUpdate },
    { "heartbeat", &GDaxLib::ProcessHeartbeat },
    { "ticker", &GDaxLib::ProcessTicker},
    { "error", &GDaxLib::ProcessError }
};

GDaxLib::GDaxLib(QObject * parent)
    : QObject(parent)
    , webSocket(Utils::QMake<QWebSocket>("webSocket", QString(), QWebSocketProtocol::VersionLatest, this))
    , pingTimer(Utils::QMake<QTimer>("pingTimer", this))
    , lastTradeId()
{
    // proxy?
    // QList<QNetworkProxy> QNetworkProxyFactory::systemProxyForQuery(const QNetworkProxyQuery &query = QNetworkProxyQuery())

    connect(webSocket, &QWebSocket::connected, this, &GDaxLib::Connected);
    connect(webSocket, &QWebSocket::textMessageReceived, this, &GDaxLib::TextMessageReceived);

    connect(webSocket, &QWebSocket::stateChanged, [&](QAbstractSocket::SocketState socketState)
    {
        log.Info(QString("WebSocket state: %1").arg(QMetaEnum::fromType<QAbstractSocket::SocketState>().valueToKey(socketState)));
        emit OnStateChanged(ToState(socketState));
    });

    connect(webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), Error);
    connect(webSocket, QOverload<const QList<QSslError> &>::of(&QWebSocket::sslErrors), SslErrors);

    connect(webSocket, &QWebSocket::pong, this, &GDaxLib::Pong);
    connect(pingTimer, &QTimer::timeout, this, &GDaxLib::Ping);
    pingTimer->start(PingTimerMs);

    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState>");
    qRegisterMetaType<ConnectedState>("ConnectedState");
    qRegisterMetaType<Tick>("Tick");

    log.Info(QString("Connecting to %1").arg(url));
    webSocket->open(QUrl(url));
}

void GDaxLib::Ping()
{
    switch (webSocket->state())
    {
        case QAbstractSocket::SocketState::UnconnectedState:
            log.Info("Reconnecting...");
            webSocket->open(QUrl(url));
            break;

        case QAbstractSocket::SocketState::ConnectedState:
            webSocket->ping();
            break;

        default:
            log.Warning(QString("Ping, WebSocket state: %1").arg(
                            QMetaEnum::fromType<QAbstractSocket::SocketState>()
                            .valueToKey(webSocket->state())));
            break;
    }
}

void GDaxLib::Connected()
{
    // need qInstallMessageHandler(SyslogMessageHandler);? and handle ourselves?
    // https://stackoverflow.com/questions/28540571/how-to-enable-and-disable-qdebug-messages
    // https://gist.github.com/polovik/10714049
    log.Info("onConnected, subscribing...");
    Clear();
    webSocket->sendTextMessage(subscribeMessage);
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

void GDaxLib::Pong()
{
    log.Spam("Pong");
}

void GDaxLib::Clear()
{
    // lock orderbook, move\improve impl
    QMutexLocker lock(&const_cast<QMutex&>(orderBook.Mutex()));
    orderBook.Clear();
    lastTradeId = 0;
}

void GDaxLib::ProcessSubscriptions(const QJsonObject & object)
{
    (void)object;
    log.Info("Subscription response");
}

void GDaxLib::ProcessError(const QJsonObject & object)
{
    // will be in the message pump, so cannot throw? need to notify ui
    QString errorMessage = object["message"].toString();
    log.Error(QString("Error message: %1").arg(errorMessage));
    throw std::runtime_error(errorMessage.toStdString());
}

void GDaxLib::ProcessSnapshot(const QJsonObject & object)
{
    Flog::ScopeLog s(log, Flog::Level::Info, "Snapshot");

    // lock orderbook, move\improve impl
    QMutexLocker lock(&const_cast<QMutex&>(orderBook.Mutex()));

    Decimal totBid;
    for (QJsonValueRef bid : object["bids"].toArray())
    {
        QJsonArray bidArray = bid.toArray();
        QString price = bidArray[0].toString();
        QString amount = bidArray[1].toString();

        Decimal dp(price.toStdString());
        Decimal da(amount.toStdString());

        orderBook.AddBid(dp, da);
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

        orderBook.AddAsk(dp, da);
        totAsk += da;
    }

    Decimal target = (totBid + totAsk) / 1000; // .1% -- move to depths chart
    orderBook.SeekRange(target);

    emit OnUpdate();
}

void GDaxLib::ProcessUpdate(const QJsonObject & object)
{
    Flog::ScopeLog s(log, Flog::Level::Spam, "Update");

    // lock orderbook, move\improve impl
    QMutexLocker lock(&const_cast<QMutex&>(orderBook.Mutex()));

    for(QJsonValueRef changes : object["changes"].toArray())
    {
        QJsonArray array = changes.toArray();
        QString  changeType = array[0].toString();
        bool bid = changeType == "buy";

        Decimal pd(array[1].toString().toStdString());
        Decimal ad(array[2].toString().toStdString());

        if (bid)
        {
            orderBook.UpdateBid(pd, ad);
        }
        else
        {
            orderBook.UpdateAsk(pd, ad);
        }
    }
    // adjust scales here?

    emit OnUpdate();
}

void GDaxLib::ProcessHeartbeat(const QJsonObject & object)
{
    Flog::ScopeLog s(log, Flog::Level::Spam, "Heartbeat");

    auto seq = static_cast<SequenceNumber>(object["sequence"].toDouble());
    auto tradeId = static_cast<TradeId>(object["last_trade_id"].toDouble());
    QString serverTimeString = object["time"].toString();

    if (lastTradeId!=0 && lastTradeId+1 != tradeId)
    {
        log.Info(QString("[%1] [%2] MissedTrade(s): %3 - %4")
                 .arg(serverTimeString)
                 .arg(seq)
                 .arg(lastTradeId+1)
                 .arg(tradeId-1));
    }

    QDateTime serverTime = QDateTime::fromString(serverTimeString, Qt::ISODateWithMs);
    emit OnHeartbeat(serverTime);
}

void GDaxLib::ProcessTicker(const QJsonObject & object)
{
    Flog::ScopeLog s(log, Flog::Level::Info, "Tick");

    auto tick(Tick::FromJson(object));
    if (tick.side == TakerSide::None)
    {
        return;
    }

    emit OnTick(tick);
}
