#include "gdaxprovider.h"

#include "websocketstream.h"
#include "orderbook.h"
#include "restprovider.h"
#include "utils.h"

#include <QThread>
#include <QtConcurrent>

namespace GDL
{
    const char defaultStreamUrl[] ="wss://ws-feed.gdax.com";
    const char defaultRestUrl[] = "https://api.gdax.com";

    Factory factory = [](Callback & callback)
    {
        return InterfacePtr(Utils::QMake<GDaxProvider>("GDaxProvider",
            defaultStreamUrl, defaultRestUrl, callback));
    };

    InterfacePtr Create(Callback & callback)
    {
        return factory(callback);
    }

    void Deleter::operator()(GDL::Interface * itf) const noexcept
    {
        delete static_cast<GDaxProvider*>(itf);
    }

    void SetFactory(const Factory & f) { factory = f; }
}

GDaxProvider::GDaxProvider(const char * streamUrl, const char * restUrl,
                           GDL::Callback & callback, QObject * parent)
    : QObject(parent)
    , callback(callback)
    , webSocketStream(Utils::QMake<WebSocketStream>("webSocketStream", streamUrl)) // no parent, move to thread
    , restProvider(Utils::QMake<RestProvider>("restProvider", restUrl, this))
    , workerThread(Utils::QMake<QThread>("QThread", this))
{
    webSocketStream->moveToThread(workerThread);
    QObject::connect(workerThread, &QThread::started, [](){ Flog::LogManager::SetThreadName("GDax"); });
    QObject::connect(workerThread, &QThread::finished, webSocketStream, &QObject::deleteLater);
    workerThread->start();

    // todo, use c++ callack impl instead of qt signals
    connect(webSocketStream, &WebSocketStream::OnSnapshot, [&]() { callback.OnSnapshot(); });
    connect(webSocketStream, &WebSocketStream::OnHeartbeat, [&](const QDateTime & serverTime) { callback.OnHeartbeat(serverTime); });
    connect(webSocketStream, &WebSocketStream::OnTick, [&](const Tick & tick) { callback.OnTick(tick); });
    connect(webSocketStream, &WebSocketStream::OnStateChanged, [&](GDL::ConnectedState state) { callback.OnStateChanged(state); });
}

void GDaxProvider::SetAuthentication(const char * key, const char * secret, const char * passphrase) const
{
    restProvider->SetAuthentication(key, secret, passphrase);
}

void GDaxProvider::ClearAuthentication() const
{
    restProvider->ClearAuthentication();
}

const OrderBook & GDaxProvider::Orders() const
{
    return webSocketStream->Orders();
}

Async<TradesResult> GDaxProvider::FetchTrades(unsigned int limit)
{
    return restProvider->FetchTrades(limit);
}

Async<CandlesResult> GDaxProvider::FetchAllCandles(Granularity granularity)
{
    return restProvider->FetchAllCandles(granularity);
}

Async<CandlesResult> GDaxProvider::FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity)
{
    return restProvider->FetchCandles(start, end, granularity);
}

Async<ServerTimeResult> GDaxProvider::FetchTime()
{
    return restProvider->FetchTime();
}

Async<OrdersResult> GDaxProvider::FetchOrders(unsigned int limit)
{
    return restProvider->FetchOrders(limit);
}

Async<OrderResult> GDaxProvider::PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side)
{
    return restProvider->PlaceOrder(size, price, side);
}

Async<CancelOrdersResult> GDaxProvider::CancelOrders()
{
    return restProvider->CancelOrders();
}

void GDaxProvider::Shutdown()
{
    workerThread->quit();
    workerThread->wait();
}



