#ifndef GDL_H
#define GDL_H

#include "iterableresult.h"
#include "genericresult.h"
#include "decimalwrap.h"

#include <memory>
#include <functional>

class QDateTime;
class OrderBook;

struct Tick;
struct Candle;
class Trade;
struct Order;

enum class MakerSide : unsigned;
enum class Granularity : unsigned;

typedef IterableResult<Trade> TradesResult;
typedef IterableResult<Candle> CandlesResult;
typedef IterableResult<Order> OrdersResult;
typedef GenericResult<Order> OrderResult;

struct JsonValueToString
{
    static QString FromJson(const QJsonValue & value)
    {
        return value.toString();
    }
};
typedef IterableResult<QString, JsonValueToString> CancelOrdersResult;

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

        virtual void FetchOrders(std::function<void(OrdersResult)> func, unsigned int limit = 0) = 0;
        virtual void PlaceOrder(std::function<void(OrderResult)> func, const Decimal & size, const Decimal & price, MakerSide side) = 0;
        virtual void CancelOrders(std::function<void(CancelOrdersResult)> func) = 0;
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
