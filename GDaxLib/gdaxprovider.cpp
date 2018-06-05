#include "gdaxprovider.h"

#include "gdaxlib.h"
#include "orderbook.h"
#include "restprovider.h"
#include "utils.h"

#include <QNetworkAccessManager>
#include <QThread>
#include <QtConcurrent>

namespace GDL
{
    InterfacePtr Create(Callback & callback)
    {
        return InterfacePtr(Utils::QMake<GDaxProvider>("GDaxProvider", callback));
    }

    void Deleter::operator ()(GDL::Interface * itf) const noexcept
    {
        static_cast<GDaxProvider*>(itf)->deleteLater(); // ?
    }
}

GDaxProvider::GDaxProvider(GDL::Callback & callback, QObject * parent)
    : QObject(parent)
    , callback(callback)
    , networkAccessManager(Utils::QMake<QNetworkAccessManager>("networkAccessManager", this))
    , gDaxLib(Utils::QMake<GDaxLib>("gDaxLib")) // no parent, move to thread
    , restProvider(Utils::QMake<RestProvider>("restProvider", networkAccessManager, this))
    , workerThread(Utils::QMake<QThread>("QThread", this))
{
    gDaxLib->moveToThread(workerThread);
    QObject::connect(workerThread, &QThread::started, [](){ Flog::LogManager::SetThreadName("GDax"); });
    QObject::connect(workerThread, &QThread::finished, gDaxLib, &QObject::deleteLater);
    workerThread->start();

    // todo, use c++ callack impl instead of qt signals
    connect(gDaxLib, &GDaxLib::OnHeartbeat, [&](const QDateTime & serverTime) { callback.OnHeartbeat(serverTime); });
    connect(gDaxLib, &GDaxLib::OnTick, [&](const Tick & tick) { callback.OnTick(tick); });
    connect(gDaxLib, &GDaxLib::OnStateChanged, [&](GDL::ConnectedState state) { callback.OnStateChanged(state); });

    //connect(restProvider, &RestProvider::OnError, [&]() { callback.OnError(); });
    connect(restProvider, &RestProvider::OnCandles, [&](std::deque<Candle> values) { callback.OnCandles(std::move(values)); });
    connect(restProvider, &RestProvider::OnTrades, [&](std::deque<Trade> values) { callback.OnTrades(std::move(values)); });
}

const OrderBook & GDaxProvider::Orders() const
{
    return gDaxLib->Orders();
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

