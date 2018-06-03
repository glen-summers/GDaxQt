#include "gdaxprovider.h"

#include "gdaxlib.h"
#include "orderbook.h"
#include "restprovider.h"
#include "utils.h"

#include <QNetworkAccessManager>
#include <QThread>
#include <QtConcurrent>

GDaxProvider::GDaxProvider(QObject * parent)
    : QObject(parent)
    , networkAccessManager(Utils::QMake<QNetworkAccessManager>("networkAccessManager", this))
    , gDaxLib(Utils::QMake<GDaxLib>("gDaxLib")) // no parent, move to thread
    , restProvider(Utils::QMake<RestProvider>("restProvider", networkAccessManager, this))
    , workerThread(Utils::QMake<QThread>("QThread", this))
{
    gDaxLib->moveToThread(workerThread);
    QObject::connect(workerThread, &QThread::finished, gDaxLib, &QObject::deleteLater);
    workerThread->start();

    // todo, use c++ callack impl instead of qt signals
    connect(gDaxLib, &GDaxLib::OnUpdate, [&]() { emit OnUpdate(); });
    connect(gDaxLib, &GDaxLib::OnHeartbeat, [&](const QDateTime & serverTime) { emit OnHeartbeat(serverTime); });
    connect(gDaxLib, &GDaxLib::OnTick, [&](const Tick & tick) { emit OnTick(tick); });
    connect(gDaxLib, &GDaxLib::OnStateChanged, [&](ConnectedState state) { emit OnStateChanged(state); });

    connect(restProvider, &RestProvider::OnError, [&]() { emit OnError(); });
    connect(restProvider, &RestProvider::OnCandles, [&](std::deque<Candle> values) { emit OnCandles(std::move(values)); });
    connect(restProvider, &RestProvider::OnTrades, [&](std::deque<Trade> values) { emit OnTrades(std::move(values)); });
}

const OrderBook & GDaxProvider::Orders() const
{
    return gDaxLib->Orders();
}

void GDaxProvider::Ping()
{
    gDaxLib->Ping();
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

