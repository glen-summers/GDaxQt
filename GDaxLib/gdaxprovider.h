#ifndef GDAXPROVIDER_H
#define GDAXPROVIDER_H

#include "candle.h"
#include "trade.h"

#include <QObject>

#include <deque>
#include <functional>

struct Tick;
class GDaxLib;
class RestProvider;
class OrderBook;

class QWebSocket;
class QJsonObject;
class QNetworkReply;
class QNetworkAccessManager;
class QThread;

class GDaxProvider : public QObject
{
    QNetworkAccessManager * const networkAccessManager;
    GDaxLib * const gDaxLib;
    RestProvider * const restProvider;
    QThread * const workerThread;

    Q_OBJECT
public:
    GDaxProvider(QObject * parent = nullptr);

    const OrderBook & Orders() const;

    void FetchTrades(unsigned int limit);
    void FetchAllCandles(Granularity granularity);
    void FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity);

    void Shutdown();
signals:
    void OnUpdate();
    void OnHeartbeat(const QDateTime & serverTime);
    void OnTick(const Tick & tick);
    void OnStateChanged(ConnectedState state);

signals:
    void OnError(); // make more specific
    void OnCandles(std::deque<Candle> values);
    void OnTrades(std::deque<Trade> values);
};

#endif // GDAXPROVIDER_H
