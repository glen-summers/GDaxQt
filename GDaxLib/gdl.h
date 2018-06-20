#ifndef GDL_H
#define GDL_H

#include "iterableresult.h"
#include "genericresult.h"
#include "asyncresult.h"

#include "decimalwrap.h"

#include <memory>
#include <functional>

class QDateTime;
class OrderBook;

struct Tick;
struct Candle;
struct Trade;
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

    // orderbook is currently managed by provider, it is updated on a worker thread
    // and provides access to a mutex to allow syncronicity. change to allow client to manage and just provide updates.
    // connection management is also handled internally, stream connection is made in constructor, a snapshot and updates
    // then arrive, a ping thread check connection and reconnects, connection updates are sent to client
    struct Interface
    {
        virtual void SetAuthentication(const char key[], const char secret[], const char passphrase[]) const = 0;
        virtual void ClearAuthentication() const = 0;

        virtual const OrderBook & Orders() const = 0;

        virtual Async<ServerTimeResult> FetchTime() = 0;
        virtual Async<TradesResult> FetchTrades(unsigned int limit) = 0;
        virtual Async<CandlesResult> FetchAllCandles(Granularity granularity) = 0;
        virtual Async<CandlesResult> FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity) = 0;
        virtual Async<OrdersResult> FetchOrders(unsigned int limit = 0) = 0;
        virtual Async<OrderResult> PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side) = 0;
        virtual Async<CancelOrdersResult> CancelOrders() = 0;

        virtual void Shutdown() = 0;
    };

    // lambdas good for connectinng async request\reponse
    // but interface seems better for streamed reponse
    struct Callback
    {
        virtual void OnSnapshot() = 0;
        virtual void OnHeartbeat(const QDateTime & serverTime) = 0;
        virtual void OnTick(const Tick & tick) = 0;
        virtual void OnStateChanged(ConnectedState state) = 0;
    };

    struct Deleter { void operator()(Interface*) const noexcept; };
    typedef std::unique_ptr<Interface, Deleter> InterfacePtr;

    InterfacePtr Create(Callback&);

    // for mocking
    typedef std::function<InterfacePtr(Callback&)> Factory;
    void SetFactory(const Factory &);
}

#endif // GDL_H
