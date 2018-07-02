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

struct Subscription;
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
    static QString FromJson(const QJsonValue & value);
};
typedef IterableResult<QString, JsonValueToString> CancelOrdersResult;

struct ServerTimeToDateTime
{
    static QDateTime FromJson(const QJsonValue & value);
};
typedef GenericResult<QDateTime, ServerTimeToDateTime> ServerTimeResult;

namespace GDL
{
    struct IStream;
    struct IRequest;
    struct IFactory;
    struct ShutDown
    {
        void operator()(IStream*);
    };

    //typedef std::unique_ptr<IStream> StreamPtr;
    typedef std::unique_ptr<IStream, ShutDown> StreamPtr;
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

    // connection management is handled internally, stream connection is made in constructor, a snapshot and updates
    // then arrive, a ping thread checks connection and reconnects, connection updates are sent to client
    struct IStream : IAuth
    {
//        virtual void Subscribe(const Subscription & subscription) = 0;
//        virtual void Unsubscribe(const Subscription & subscription) = 0;
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

    struct OrderBookItem
    {
        Decimal price, amount;

        static OrderBookItem FromJson(const QJsonValue & value);
    };
    typedef IterableResult<OrderBookItem> OrderBookItemIter;

    struct OrderBookChange
    {
        MakerSide side;
        Decimal price, amount;

        static OrderBookChange FromJson(const QJsonValue & value);
    };
    typedef IterableResult<OrderBookItem> OrderBookItemIter;

    // lambdas good for connectinng async request\reponse
    // but interface better for streamed reponse
    // but itf may be too obtrusive if add too many types, how about a single OnMessage(type, opaque data) + swicth Get<T>(), or reqister individual callbacks?
    struct IStreamCallbacks
    {
        virtual void OnSnapshot(const QString & product, const IterableResult<OrderBookItem> & bids, const IterableResult<OrderBookItem> & asks) = 0;
        virtual void OnUpdate(const QString & product, const IterableResult<OrderBookChange> & changes) = 0;
        virtual void OnHeartbeat(const QDateTime & serverTime) = 0;
        virtual void OnTick(const Tick & tick) = 0;
        virtual void OnStateChanged(ConnectedState state) = 0;

    protected:
        virtual ~IStreamCallbacks() = default;
    };

    // would like...
    // create an unauthorised subscription stream, get callbacks
    // create an authorised subscription stream, get callbacks
    // create an unauthorised request object, standalone? callbacks nullable?
    // create an authorised request object
    // set authorisation on factory? currently have one factory prefer not to store credentials statically
    // add CreateFactory, with auth override?
    // how set sandbox? parallel sandboxFactory?
    // client should call Ping on its own timer and handle reconnection
    // subscribe should have an async lambda for success\error
    // async call timeouts??
    // factory to create authenticator
    // or set directly on stream/request
    // have setter for callbacks? multicast boost::signals2

    // requirements
    // set auth as a single action that provides auth for all calls on that object tree
    // allow multiple authenticators without interference, dont have setter on factory while multiple getters could run
    // so set auth on factory subobject(s)?
    // would like to be able to create separate stream and request objects
    // but maybe a single one is better but requires a callback, unless optional
    // when does websocket get connected? has 5 second inactivity error, create on demand in stream getters?
    // cleaner in ctor, but what id only doing rest calls :(
    // could have a callbackSetter as web socket connect will not invoke it, subscribe will
    // InterfacePtr itf =  Create();
    // itf.Callback+= this;
    // itf.Subscribe(Async<Result>);


    struct IFactory
    {
        //virtual AuthPtr CreateAuthenticator(/*creds*/) const = 0;
        virtual StreamPtr CreateStream(IStreamCallbacks &, const Subscription &) const = 0; // +optional authPtr?
        virtual RequestPtr CreateRequest(const char * product) const = 0; // +optional authPtr?
        virtual ~IFactory() = default;
    };

    const IFactory & GetFactory();
    void SetFactory(FactoryPtr);
    void SetSandbox();
}

#endif // GDL_H
