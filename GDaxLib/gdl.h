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
    struct IStream;
    struct IRequest;
    struct IFactory;

    typedef std::unique_ptr<IStream> StreamPtr;
    typedef std::unique_ptr<IRequest> RequestPtr;
    typedef std::unique_ptr<IFactory> FactoryPtr;

    enum class ConnectedState : unsigned
    {
        NotConnected,
        Connecting,
        Connected,
    };

    struct IAuth
    {
        // change sig, to IAuthentor? have one instance?
        virtual void SetAuthentication(const char key[], const char secret[], const char passphrase[]) = 0;
        virtual void ClearAuthentication() = 0;
    protected:
        virtual ~IAuth() = default;
    };

    // orderbook is currently managed by provider, it is updated on a worker thread
    // and provides access to a mutex to allow syncronicity. change to allow client to manage and just provide updates.
    // connection management is also handled internally, stream connection is made in constructor, a snapshot and updates
    // then arrive, a ping thread check connection and reconnects, connection updates are sent to client
    struct IStream : IAuth
    {
        virtual const OrderBook & Orders() const = 0;
        virtual void Shutdown() = 0;
        virtual ~IStream() = default;
    };

    struct IRequest : IAuth
    {
        virtual Async<ServerTimeResult> FetchTime() = 0;
        virtual Async<TradesResult> FetchTrades(unsigned int limit) = 0;
        virtual Async<CandlesResult> FetchAllCandles(Granularity granularity) = 0;
        virtual Async<CandlesResult> FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity) = 0;
        virtual Async<OrdersResult> FetchOrders(unsigned int limit = 0) = 0;
        virtual Async<OrderResult> PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side) = 0;
        virtual Async<CancelOrdersResult> CancelOrders() = 0;
        virtual ~IRequest() = default;
    };

    // lambdas good for connectinng async request\reponse
    // but interface seems better for streamed reponse
    struct IStreamCallbacks
    {
        virtual void OnSnapshot() = 0;
        virtual void OnHeartbeat(const QDateTime & serverTime) = 0;
        virtual void OnTick(const Tick & tick) = 0;
        virtual void OnStateChanged(ConnectedState state) = 0;
    protected:
        virtual ~IStreamCallbacks() = default;
    };

    struct IFactory
    {
        virtual StreamPtr CreateStream(IStreamCallbacks&) const = 0;
        virtual RequestPtr CreateRequest() const = 0;
        virtual ~IFactory() = default;
    };

    const IFactory & GetFactory();
    void SetFactory(FactoryPtr);
    void SetSandbox();
}

#endif // GDL_H
