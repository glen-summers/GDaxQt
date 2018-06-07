#ifndef GDL_H
#define GDL_H

#include "candle.h"
#include "trade.h"

#include <QNetworkReply>

#include <deque>
#include <memory>
#include <functional>

struct Tick;
class QDateTime;
class OrderBook;

namespace GDL
{
    extern const char defaultStreamUrl[];
    extern const char defaultRestUrl[];

    enum class ConnectedState : int
    {
        NotConnected,
        Connecting,
        Connected,
    };

    struct Interface
    {
        virtual const OrderBook & Orders() const = 0;
        virtual void FetchTrades(unsigned int limit) = 0;
        virtual void FetchAllCandles(Granularity granularity) = 0;
        virtual void FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity) = 0;
        virtual void Shutdown() = 0;
    };

    struct Callback
    {
        virtual void OnHeartbeat(const QDateTime & serverTime) = 0;
        virtual void OnTick(const Tick & tick) = 0;
        virtual void OnStateChanged(ConnectedState state) = 0;
        virtual void OnCandles(std::deque<Candle> values) = 0;
        virtual void OnTrades(std::deque<Trade> values) = 0;
    };

    struct Deleter { void operator()(Interface*) const noexcept; };
    typedef std::unique_ptr<Interface, Deleter> InterfacePtr;

    InterfacePtr Create(Callback&);

    // for mocking
    typedef std::function<InterfacePtr(Callback&)> Factory;
    void SetFactory(const Factory &);
}

#endif // GDL_H
