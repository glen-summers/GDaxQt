#include "restprovider.h"

#include "defs.h"
#include "utils.h"
#include "authenticator.h"

#include "order.h"

#include "flogging.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValueRef>
#include <QMetaEnum>
#include <QNetworkCookieJar>
#include <QStringBuilder>
#include <QUrlQuery>
#include <QMessageAuthenticationCode>

#include <cassert>

namespace
{
    const Flog::Log flog = Flog::LogManager::GetLog<RestProvider>();

    constexpr const char Products[] = "/products/";
    constexpr const char Candles[] = "/candles";
    constexpr const char Trades[] = "/trades";
    constexpr const char Orders[] = "/orders";
    constexpr const char Time[] = "/time";

    constexpr const char CbAccessKey[]= "CB-ACCESS-KEY";
    constexpr const char CbAccessSign[] = "CB-ACCESS-SIGN";
    constexpr const char CbAccessTimestamp[] = "CB-ACCESS-TIMESTAMP";
    constexpr const char CbAccessPassphrase[] = "CB-ACCESS-PASSPHRASE";
}

RestProvider::RestProvider(const char * baseUrl, const char * product, QObject * parent)
    : QObject(parent)
    , baseUrl(baseUrl)
    , product(product)
    , manager(Utils::QMake<QNetworkAccessManager>("networkAccessManager", this))
{
}

// prevents ~UniquePtr compile error with incomplete type
RestProvider::~RestProvider() = default;

void RestProvider::SetAuthentication(const char key[], const char secret[], const char passphrase[])
{
    authenticator = std::make_unique<Authenticator>(key, QByteArray::fromBase64(secret), passphrase);
}

void RestProvider::ClearAuthentication()
{
    authenticator.reset();
}

Async<ServerTimeResult> RestProvider::FetchTime()
{
    return CreateRequest(false, RequestMethod::Get, baseUrl % Time);
}

Async<TradesResult> RestProvider::FetchTrades(unsigned int limit)
{
    QUrlQuery query;
    query.addQueryItem("limit", QString::number(limit));
    return CreateRequest(false, RequestMethod::Get, baseUrl % Products % product % Trades, &query);
}

// FetchCandles seems flakey on the server, if we request with an endtime > server UTC time then
// both parameters are ignored and the last 300 candles are returned
Async<CandlesResult> RestProvider::FetchAllCandles(Granularity granularity)
{
    QUrlQuery query;
    query.addQueryItem("granularity", QString::number(static_cast<unsigned int>(granularity)));
    return CreateRequest(false, RequestMethod::Get, baseUrl % Products % product % Candles, &query);
}

Async<CandlesResult> RestProvider::FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity)
{
    QUrlQuery query;
    query.addQueryItem("start", start.toString(Qt::ISODate));
    query.addQueryItem("end", end.toString(Qt::ISODate));
    query.addQueryItem("granularity", QString::number(static_cast<unsigned int>(granularity)));
    return CreateRequest(false, RequestMethod::Get, baseUrl % Products % product % Candles, &query);
}

Async<OrdersResult> RestProvider::FetchOrders(unsigned int limit)
{
    QUrlQuery query;
    if (limit != 0)
    {
        query.addQueryItem("limit", QString::number(limit));
    }

    return CreateRequest(true, RequestMethod::Get, baseUrl % Orders, &query);
}

Async<OrderResult> RestProvider::PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side)
{
    QString siderian = MakerSideToString(side);
    QString sz = DecNs::toString(size).c_str();
    QString pr = DecNs::toString(price).c_str();

    QJsonDocument doc(QJsonObject
    {
        // class
        {"price", pr},
        {"size", sz},
        {"side", siderian },
        {"product_id", product}

        // +client_oid
        // type: limit*|market
        // stp
        //stop: loss|entry, requires stop_price
        //stop_price
    });
    auto data = doc.toJson();
    return CreateRequest(true, RequestMethod::Post, baseUrl % Orders, {}, &data);
}

Async<CancelOrdersResult> RestProvider::CancelOrders()
{
    // + optional product_id
    return CreateRequest(true, RequestMethod::Delete, baseUrl % Orders);
}

void RestProvider::Authenticate(QNetworkRequest & request, const QString & httpMethod, const QByteArray * body) const
{
    if (!authenticator)
    {
        throw std::logic_error("Authentication has not been configured");
    }

    auto timestamp = QString::number(QDateTime::currentSecsSinceEpoch()); // needs to get from server if time drift +-30s!
    auto pathForSignature = request.url().toString(QUrl::RemoveScheme | QUrl::RemoveAuthority);
    QByteArray signature = authenticator->ComputeSignature(httpMethod, timestamp, pathForSignature, body);
    request.setRawHeader(QByteArray(CbAccessKey), authenticator->ApiKey());
    request.setRawHeader(QByteArray(CbAccessSign), signature);
    request.setRawHeader(QByteArray(CbAccessTimestamp), timestamp.toUtf8());
    request.setRawHeader(QByteArray(CbAccessPassphrase), authenticator->Passphrase());
}

QNetworkReply * RestProvider::CreateRequest(bool authenticate, RequestMethod method, const QString & requestPath,
                                            const QUrlQuery * query, const QByteArray * body) const
{
    QString httpMethod = RequestMethodToString(method);
    QUrl url(requestPath);
    if (query)
    {
        url.setQuery(*query);
    }

    flog.Info("{0}Request {1} : {2}", authenticate?"Authenticated":"", httpMethod, url.toString());

    QNetworkRequest request(url);
    if (authenticate)
    {
        Authenticate(request, httpMethod, body);
    }

    // set agent?
    QNetworkReply * reply = nullptr;
    switch (method)
    {
        case RequestMethod::Get:
            reply = manager->get(request);
            break;

        case RequestMethod::Post:
            request.setRawHeader("Content-Type", "application/json");
            reply = manager->post(request, body ? *body : QByteArray{});
            break;

        case RequestMethod::Delete:
            reply = manager->deleteResource(request);
            break;

        default:;
    }
    if (!reply)
    {
        throw std::logic_error("invalid method");
    }

    Utils::Detail::Constructed("reply", reply);

    return reply;
}

QString RestProvider::RequestMethodToString(RestProvider::RequestMethod method)
{
    switch (method)
    {
        case RequestMethod::Get: return "GET";
        case RequestMethod::Post: return "POST";
        case RequestMethod::Delete: return "DELETE";
        default:;
    }
    throw std::logic_error("invalid method");
}
