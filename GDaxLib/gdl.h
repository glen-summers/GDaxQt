#ifndef GDL_H
#define GDL_H

#include "candle.h"
#include "trade.h"
#include "order.h"
#include "jsonarrayresult.h"
#include "genericresult.h"

#include <memory>
#include <functional>

// namespace?
struct Tick;
class QDateTime;
class OrderBook;

// todo, try and expose these by a base interface?
typedef JsonArrayResult<Trade> TradesResult;
typedef JsonArrayResult<Candle> CandlesResult;
typedef JsonArrayResult<Order> OrdersResult;
typedef GenericResult<Order> OrderResult;
struct JsonValueToString
{
    static QString FromJson(const QJsonValue & value)
    {
        return value.toString();
    }
};
typedef JsonArrayResult<QString, JsonValueToString> CancelOrdersResult;

struct ServerTimeToDateTime
{
    static QDateTime FromJson(const QJsonValue & value)
    {
        return QDateTime::fromString(value["iso"].toString(), Qt::DateFormat::ISODateWithMs);
    }
};
typedef GenericResult<QDateTime, ServerTimeToDateTime> ServerTimeResult;

namespace GDL
{
    extern const char defaultStreamUrl[];
    extern const char defaultRestUrl[];

    enum class ConnectedState : unsigned
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
        virtual void OnSnapshot() = 0;
        virtual void OnHeartbeat(const QDateTime & serverTime) = 0;
        virtual void OnTick(const Tick & tick) = 0;
        virtual void OnStateChanged(ConnectedState state) = 0;
        virtual void OnCandles(const CandlesResult & values) = 0;
        virtual void OnTrades(const TradesResult & values) = 0;
    };

    struct Deleter { void operator()(Interface*) const noexcept; };
    typedef std::unique_ptr<Interface, Deleter> InterfacePtr;

    InterfacePtr Create(Callback&);

    // for mocking
    typedef std::function<InterfacePtr(Callback&)> Factory;
    void SetFactory(const Factory &);
}

#endif // GDL_H
