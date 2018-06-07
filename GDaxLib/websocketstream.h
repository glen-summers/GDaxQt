#ifndef WEBSOCKETSTREAM_H
#define WEBSOCKETSTREAM_H

#include "defs.h"
#include "orderbook.h"
#include "gdl.h" //fwd?

#include "flogging.h"

#include <QObject>
#include <unordered_map>

struct Tick;
class QWebSocket;
class QJsonObject;
class QTimer;

class WebSocketStream : public QObject
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<WebSocketStream>();

    Q_OBJECT

    typedef std::unordered_map<std::string, void(WebSocketStream::*)(const QJsonObject & object)> FunctionMap;
    static FunctionMap functionMap;

    QString const url;
    QWebSocket * const webSocket;
    QTimer * const pingTimer;
    OrderBook orderBook;
    TradeId lastTradeId;

public:
    explicit WebSocketStream(const char * url, QObject * parent = nullptr);

    const OrderBook & Orders() const { return orderBook; }

signals:
    void OnHeartbeat(const QDateTime & serverTime);
    void OnTick(const Tick & tick);
    void OnStateChanged(GDL::ConnectedState state);

private slots:
    void Connected();
    void TextMessageReceived(QString message);
    void Ping();
    void Pong();

private:
    void Clear();
    void ProcessSubscriptions(const QJsonObject & object);
    void ProcessError(const QJsonObject & object);
    void ProcessSnapshot(const QJsonObject & object);
    void ProcessUpdate(const QJsonObject & object);
    void ProcessHeartbeat(const QJsonObject & object);
    void ProcessTicker(const QJsonObject & object);
};

#endif // WEBSOCKETSTREAM_H
