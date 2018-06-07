#ifndef GDAXPROVIDER_H
#define GDAXPROVIDER_H

#include "candle.h"
#include "trade.h"
#include "gdl.h"

#include <QObject>

#include <deque>
#include <functional>

struct Tick;
class WebSocketStream;
class RestProvider;

class QWebSocket;
class QJsonObject;
class QNetworkReply;
class QNetworkAccessManager;
class QThread;

class GDaxProvider : public QObject, public GDL::Interface
{
    GDL::Callback & callback;
    QNetworkAccessManager * const networkAccessManager;
    WebSocketStream * const webSocketStream;
    RestProvider * const restProvider;
    QThread * const workerThread;

    Q_OBJECT
public:
    GDaxProvider(const char * streamUrl, const char * restUrl,
            GDL::Callback & callback, QObject * parent = nullptr);

    const OrderBook & Orders() const;

    void FetchTrades(unsigned int limit);
    void FetchAllCandles(Granularity granularity);
    void FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity);
    void Shutdown();
};

#endif // GDAXPROVIDER_H
