#ifndef GDAXPROVIDER_H
#define GDAXPROVIDER_H

#include "gdl.h"

#include <QObject>

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

    void SetAuthentication(const char * key, const char * secret, const char * passphrase) const override;
    void ClearAuthentication() const override;

    const OrderBook & Orders() const override;

    Async<ServerTimeResult> FetchTime() override;
    Async<TradesResult> FetchTrades(unsigned int limit) override;
    Async<CandlesResult> FetchAllCandles(Granularity granularity) override;
    Async<CandlesResult> FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity) override;
    Async<OrdersResult> FetchOrders(unsigned int limit = 0) override;
    Async<OrderResult> PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side) override;
    Async<CancelOrdersResult> CancelOrders() override;

    void Shutdown() override;
};

#endif // GDAXPROVIDER_H
