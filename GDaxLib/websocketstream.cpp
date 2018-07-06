#include "websocketstream.h"

#include "tick.h"
#include "utils.h"
#include "authenticator.h"
#include "subscription.h"

#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QMetaEnum>
#include <QNetworkReply>
#include <QTimer>
#include <QCoreApplication>

namespace
{
    constexpr bool useWorkerThead = true;

    constexpr int PingTimerMs = 5000;

    const Flog::Log log = Flog::LogManager::GetLog<WebSocketStream>();

    // informational atm
    void Error(QAbstractSocket::SocketError error)
    {
        const char * err = QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error);
        log.Error("SocketError: {0}", err ? err : "unknown");
    }

    void SslErrors(QList<QSslError> errors)
    {
        for(auto & e : errors)
        {
            log.Error("SslError: {0}, {1}", e.error(), e.errorString());
        }
    }

    GDL::ConnectedState ToState(QAbstractSocket::SocketState socketState)
    {
        switch (socketState)
        {
            case QAbstractSocket::ConnectingState:
                return GDL::ConnectedState::Connecting;

            case QAbstractSocket::ConnectedState:
                return GDL::ConnectedState::Connected;

            default:
                return GDL::ConnectedState::NotConnected;;
        };
    }

    QString ToJson(const Subscription & subscription, bool subscribe)
    {
        QJsonObject obj;
        obj.insert("type", subscribe? "subscribe" : "unsubscribe");

        QJsonArray products, channels;
        for (auto & p : subscription.ProductIds)
        {
            products.push_back(p.c_str());
        }
        obj.insert("product_ids", products);
        for (auto & c : subscription.Channels)
        {
            channels.push_back(c.c_str());
        }
        obj.insert("channels", channels);
        return QJsonDocument(obj).toJson();
    }
}

WebSocketStream::FunctionMap WebSocketStream::functionMap =
{
    { "subscriptions", &WebSocketStream::ProcessSubscriptions },
    { "snapshot", &WebSocketStream::ProcessSnapshot },
    { "l2update", &WebSocketStream::ProcessUpdate },
    { "heartbeat", &WebSocketStream::ProcessHeartbeat },
    { "ticker", &WebSocketStream::ProcessTicker},
    { "error", &WebSocketStream::ProcessError },

// last_match
//    { "received" , &WebSocketStream::ProcessReceived },
//    { "open" , &WebSocketStream::ProcessOpen },
//    { "done" , &WebSocketStream::ProcessDone },
};

WebSocketStream::WebSocketStream(const char * url, const Subscription & subscription, GDL::IStreamCallbacks & callback, GDL::Auth * auth)
    : callback(callback)
    , url(url)
    , subscription(subscription)
    , authenticator(Authenticator::Create(auth))
    , webSocket(Utils::QMake<QWebSocket>("webSocket", QString(), QWebSocketProtocol::VersionLatest, this))
    , workerThread(Utils::QMake<QThread>("workerThread"))
    , pingTimer(Utils::QMake<QTimer>("pingTimer", this))
    , lastTradeId()
{
    // proxy?
    // QList<QNetworkProxy> QNetworkProxyFactory::systemProxyForQuery(const QNetworkProxyQuery &query = QNetworkProxyQuery())

    connect(webSocket, &QWebSocket::connected, this, &WebSocketStream::Connected);
    connect(webSocket, &QWebSocket::textMessageReceived, this, &WebSocketStream::TextMessageReceived);
    connect(webSocket, &QWebSocket::stateChanged, [&](QAbstractSocket::SocketState socketState)
    {
        log.Info("WebSocket state: {0}", QMetaEnum::fromType<QAbstractSocket::SocketState>().valueToKey(socketState));
        callback.OnStateChanged(ToState(socketState));
    });
    // need to marshall errors register meta
    connect(webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), Error);
    connect(webSocket, QOverload<const QList<QSslError> &>::of(&QWebSocket::sslErrors), SslErrors);

    connect(webSocket, &QWebSocket::pong, this, &WebSocketStream::Pong);
    connect(pingTimer, &QTimer::timeout, this, &WebSocketStream::Ping);

    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState>");
    qRegisterMetaType<GDL::ConnectedState>("GDL::ConnectedState");
    qRegisterMetaType<Tick>("Tick");

    Open();

    if (useWorkerThead)
    {
        this->moveToThread(workerThread);
        QObject::connect(workerThread, &QThread::started, [this]()
        {
            Flog::LogManager::SetThreadName("GDax");
            pingTimer->start(PingTimerMs);
        });
        QObject::connect(workerThread, &QThread::finished, [this]()
        {
            this->deleteLater();
            workerThread->deleteLater(); // still gets logged as leak
        });
        workerThread->start();
    } // else setParents for delete
}

// prevents ~UniquePtr compile error with incomplete type
WebSocketStream::~WebSocketStream() = default;

// these need to added to a queue and processed sequentially and honouring connected status
// and\OR to merge into a single held subcription so can resubscribe on connection errors

//void WebSocketStream::Subscribe(const Subscription & subscription)
//{
//    webSocket->sendBinaryMessage(ToJson(subscription, true));
//}

//void WebSocketStream::Unsubscribe(const Subscription & subscription)
//{
//    webSocket->sendBinaryMessage(ToJson(subscription, false));
//}

void WebSocketStream::Shutdown()
{
    if (useWorkerThead)
    {
        workerThread->quit();
        workerThread->wait();
    }
}

void WebSocketStream::Ping()
{
    switch (webSocket->state())
    {
        case QAbstractSocket::SocketState::UnconnectedState:
            Open();
            break;

        case QAbstractSocket::SocketState::ConnectedState:
            webSocket->ping();
            break;

        default:
            log.Warning("Ping, WebSocket state: {0}", QMetaEnum::fromType<QAbstractSocket::SocketState>()
                        .valueToKey(webSocket->state()));
            break;
    }
}

void WebSocketStream::Connected()
{
    // need qInstallMessageHandler(SyslogMessageHandler);? and handle ourselves?
    // https://stackoverflow.com/questions/28540571/how-to-enable-and-disable-qdebug-messages
    // https://gist.github.com/polovik/10714049
    log.Info("onConnected");
    Clear(); // avoid state?

    webSocket->sendTextMessage(ToJson(subscription, true));
}

void WebSocketStream::TextMessageReceived(QString message)
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
            log.Warning("Unprocessed message: {0} : {1}", type, message);
        }
    }
    catch (const std::exception & e)
    {
        log.Error("Error: {0}, {1}", e.what(), message);
    }
}

void WebSocketStream::Pong()
{
    log.Spam("Pong");
}

void WebSocketStream::Open()
{
    log.Info("Connecting to {0}", url);

    // todo: consolidate
    constexpr const char CbAccessKey[]= "CB-ACCESS-KEY";
    constexpr const char CbAccessSign[] = "CB-ACCESS-SIGN";
    constexpr const char CbAccessTimestamp[] = "CB-ACCESS-TIMESTAMP";
    constexpr const char CbAccessPassphrase[] = "CB-ACCESS-PASSPHRASE";

    QNetworkRequest request(url);
    if (authenticator)
    {
        // as GET /users/self/verify
        auto timestamp = QString::number(QDateTime::currentSecsSinceEpoch()); // needs to get from server if time drift +-30s!
        auto pathForSignature = "/users/self/verify";
        QByteArray signature = authenticator->ComputeSignature("GET", timestamp, pathForSignature, nullptr);
        request.setRawHeader(QByteArray(CbAccessKey), authenticator->ApiKey());
        request.setRawHeader(QByteArray(CbAccessSign), signature);
        request.setRawHeader(QByteArray(CbAccessTimestamp), timestamp.toUtf8());
        request.setRawHeader(QByteArray(CbAccessPassphrase), authenticator->Passphrase());
        // this only works occasionally, dont always get received\open order messages??
    }
    webSocket->open(request);
}

void WebSocketStream::Clear()
{
    lastTradeId = 0;
}

void WebSocketStream::ProcessSubscriptions(const QJsonObject & object)
{
    (void)object;
    log.Info("Subscription response");
}

void WebSocketStream::ProcessError(const QJsonObject & object)
{
    // will be in the message pump, so cannot throw? need to notify ui
    QString errorMessage = object["message"].toString();
    log.Error("Error message: {0}", errorMessage);
    throw std::runtime_error(errorMessage.toStdString());
}

void WebSocketStream::ProcessSnapshot(const QJsonObject & object)
{
    callback.OnSnapshot("todo Product", object["bids"].toArray(), object["asks"].toArray());
}

void WebSocketStream::ProcessUpdate(const QJsonObject & object)
{
    callback.OnUpdate("todo Product", object["changes"].toArray());
}

void WebSocketStream::ProcessHeartbeat(const QJsonObject & object)
{
    Flog::ScopeLog s(log, Flog::Level::Spam, "Heartbeat");

    auto seq = static_cast<SequenceNumber>(object["sequence"].toDouble());
    auto tradeId = static_cast<TradeId>(object["last_trade_id"].toDouble());
    QString serverTimeString = object["time"].toString();

    if (lastTradeId!=0 && lastTradeId+1 != tradeId)
    {
        log.Info("[{0}] [{1}] MissedTrade(s): {2} - {3}",
                 serverTimeString, seq, lastTradeId+1, tradeId-1);
    }

    QDateTime serverTime = QDateTime::fromString(serverTimeString, Qt::ISODateWithMs);
    callback.OnHeartbeat(serverTime);
}

void WebSocketStream::ProcessTicker(const QJsonObject & object)
{
    Flog::ScopeLog s(log, Flog::Level::Info, "Tick");

    auto tick(Tick::FromJson(object));
    if (tick.side == TakerSide::None)
    {
        return;
    }

    callback.OnTick(tick);
}

//struct OrderUpdate // differnt from rest order
//{
//    QString id; // order_id
//    OrderType type; // order_type
//    Decimal size;
//    Decimal price;
//    MakerSide side;
//    QString productId;
//    SequenceNumber sequence;
//    QDateTime time;
//};

void WebSocketStream::ProcessReceived(const QJsonObject &object)
{
    /* + client_oid
{
    "order_id":"f752b833-f468-46eb-b5e8-e67b137c49b6",
    "order_type":"limit",
    "size":"0.01000000",
    "price":"0.10000000",
    "side":"buy",
    "product_id":"BTC-EUR",
    "sequence":6825716,
    "time":"2018-07-01T10:26:53.533000Z"
}
*/
}

void WebSocketStream::ProcessOpen(const QJsonObject &object)
{
/*same as update, missing order_type
{
    "side":"buy",
    "price":"0.10000000",
    "order_id":"f752b833-f468-46eb-b5e8-e67b137c49b6",
    "remaining_size":"0.01000000",
    "product_id":"BTC-EUR",
    "sequence":6825717,
    "time":"2018-07-01T10:26:53.533000Z"
}
*/
}

void WebSocketStream::ProcessDone(const QJsonObject &object)
{
/* same as received, + reason
{
    "side":"buy",
    "order_id":"f752b833-f468-46eb-b5e8-e67b137c49b6",
    "reason":"canceled",
    "product_id":"BTC-EUR",
    "price":"0.10000000",
    "remaining_size":"0.01000000",
    "sequence":6825718,
    "time":"2018-07-01T10:27:03.293000Z"
}
*/
}
