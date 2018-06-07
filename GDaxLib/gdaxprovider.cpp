#include "gdaxprovider.h"

#include "websocketstream.h"
#include "orderbook.h"
#include "restprovider.h"
#include "utils.h"

#include <QNetworkAccessManager>
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
    , networkAccessManager(Utils::QMake<QNetworkAccessManager>("networkAccessManager", this))
    , webSocketStream(Utils::QMake<WebSocketStream>("webSocketStream", streamUrl)) // no parent, move to thread
    , restProvider(Utils::QMake<RestProvider>("restProvider", restUrl, networkAccessManager, this))
    , workerThread(Utils::QMake<QThread>("QThread", this))
{
    webSocketStream->moveToThread(workerThread);
    QObject::connect(workerThread, &QThread::started, [](){ Flog::LogManager::SetThreadName("GDax"); });
    QObject::connect(workerThread, &QThread::finished, webSocketStream, &QObject::deleteLater);
    workerThread->start();

    // todo, use c++ callack impl instead of qt signals
    connect(webSocketStream, &WebSocketStream::OnHeartbeat, [&](const QDateTime & serverTime) { callback.OnHeartbeat(serverTime); });
    connect(webSocketStream, &WebSocketStream::OnTick, [&](const Tick & tick) { callback.OnTick(tick); });
    connect(webSocketStream, &WebSocketStream::OnStateChanged, [&](GDL::ConnectedState state) { callback.OnStateChanged(state); });

    //connect(restProvider, &RestProvider::OnError, [&]() { callback.OnError(); });
    connect(restProvider, &RestProvider::OnCandles, [&](std::deque<Candle> values) { callback.OnCandles(std::move(values)); });
    connect(restProvider, &RestProvider::OnTrades, [&](std::deque<Trade> values) { callback.OnTrades(std::move(values)); });
}

const OrderBook & GDaxProvider::Orders() const
{
    return webSocketStream->Orders();
}

void GDaxProvider::FetchTrades(unsigned int limit)
{
    restProvider->FetchTrades(limit);
}

void GDaxProvider::FetchAllCandles(Granularity granularity)
{
    restProvider->FetchAllCandles(granularity);
}

void GDaxProvider::FetchCandles(const QDateTime &start, const QDateTime &end, Granularity granularity)
{
    return restProvider->FetchCandles(start, end, granularity);
}

void GDaxProvider::Shutdown()
{
    workerThread->quit();
    workerThread->wait();
}

