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

    // parm
    constexpr const char Product[] = "BTC-EUR";

    constexpr const char Products[] = "/products/";
    constexpr const char Candles[] = "/candles";
    constexpr const char Trades[] = "/trades";
    constexpr const char Orders[] = "/orders";
    constexpr const char Time[] = "/time";

    constexpr const char CbAccessKey[]= "CB-ACCESS-KEY";
    constexpr const char CbAccessSign[] = "CB-ACCESS-SIGN";
    constexpr const char CbAccessTimestamp[] = "CB-ACCESS-TIMESTAMP";
    constexpr const char CbAccessPassphrase[] = "CB-ACCESS-PASSPHRASE";

    template <typename T>
    void WhenFinished(QNetworkReply * reply, T func)
    {
        reply->ignoreSslErrors(); // diags, set from config
        QObject::connect(reply, &QNetworkReply::finished, [func{std::move(func)}, reply]()
        {
            func(reply);
            reply->deleteLater();
        });
    }
}

RestProvider::RestProvider(const char * baseUrl, QNetworkAccessManager * manager, QObject * parent)
    : QObject(parent)
    , baseUrl(baseUrl)
    , manager(manager)
{
}

// prevents ~UniquePtr compile error with incomplete type
RestProvider::~RestProvider() = default;

void RestProvider::SetAuthentication(QByteArray apiKey, QByteArray secretKey, QByteArray passphrase)
{
    authenticator = std::make_unique<Authenticator>(apiKey, QByteArray::fromBase64(secretKey), passphrase);
}

void RestProvider::ClearAuthentication()
{
    authenticator.reset();
}

Async<ServerTimeResult> RestProvider::FetchTime()
{
    QNetworkRequest request = CreateRequest(baseUrl % Time, {});
    QNetworkReply * reply = manager->get(request);
    return reply;
}

Async<TradesResult> RestProvider::FetchTrades(unsigned int limit)
{
    QUrlQuery query;
    query.addQueryItem("limit", QString::number(limit));

    QUrl url(baseUrl % Products % Product % Trades);
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply * reply = manager->get(request);
    return reply;
}

// FetchCandles seems flakey on the server, if we request with an endtime > server UTC time then
// both parameters are ignored and the last 300 candles are returned
Async<CandlesResult> RestProvider::FetchAllCandles(Granularity granularity)
{
    QUrlQuery query;
    query.addQueryItem("granularity", QString::number(static_cast<unsigned int>(granularity)));
    QNetworkRequest request = CreateRequest(baseUrl % Products % Product % Candles, query);
    QNetworkReply * reply = manager->get(request);
    return reply;
}

Async<CandlesResult> RestProvider::FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity)
{
    QUrlQuery query;
    query.addQueryItem("start", start.toString(Qt::ISODate));
    query.addQueryItem("end", end.toString(Qt::ISODate));
    query.addQueryItem("granularity", QString::number(static_cast<unsigned int>(granularity)));
    QNetworkRequest request = CreateRequest(baseUrl % Products % Product % Candles, query);
    QNetworkReply * reply = manager->get(request);
    return reply;
}

Async<OrdersResult> RestProvider::FetchOrders(unsigned int limit)
{
    QUrlQuery query;
    if (limit != 0)
    {
        query.addQueryItem("limit", QString::number(limit));
    }

    QNetworkRequest request = CreateAuthenticatedRequest("GET", Orders, query, {});
    QNetworkReply * reply = manager->get(request);
    return reply;
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
        {"product_id", Product}

        // +client_oid
        // type: limit*|market
        // stp
        //stop: loss|entry, requires stop_price
        //stop_price
    });
    auto data = doc.toJson();

    QNetworkRequest request = CreateAuthenticatedRequest("POST", Orders, {}, data);
    request.setRawHeader("Content-Type", "application/json");
    QNetworkReply * reply = manager->post(request, data);
    return reply;
}

Async<CancelOrdersResult> RestProvider::CancelOrders()
{
    // + optional product_id
    QNetworkRequest request = CreateAuthenticatedRequest("DELETE", Orders, {}, {});
    QNetworkReply * reply = manager->deleteResource(request);
    return reply;
}

QNetworkRequest RestProvider::CreateAuthenticatedRequest(const QString & httpMethod, const QString & requestPath, const QUrlQuery & query,
                                                         const QByteArray & body) const
{
    if (!authenticator)
    {
        throw std::logic_error("Authentication has not been configured");
    }

    QUrl url(baseUrl % requestPath);
    url.setQuery(query);

    auto timestamp = QString::number(QDateTime::currentSecsSinceEpoch()); // needs to get from server if time drift +-30s!
    auto pathForSignature = url.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority);
    QByteArray signature = authenticator->ComputeSignature(httpMethod, timestamp, pathForSignature, body);

    flog.Info("Authenticated Request {0} : {1}", httpMethod, url.toString());
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray(CbAccessKey), authenticator->ApiKey());
    request.setRawHeader(QByteArray(CbAccessSign), signature);
    request.setRawHeader(QByteArray(CbAccessTimestamp), timestamp.toUtf8());
    request.setRawHeader(QByteArray(CbAccessPassphrase), authenticator->Passphrase());
    // set agent?
    return request;
}

QNetworkRequest RestProvider::CreateRequest(const QString & requestPath, const QUrlQuery & query) const
{
    QUrl url(requestPath);
    url.setQuery(query);
    flog.Info("Request {0}", url.toString());
    QNetworkRequest request(url);
    // set agent?
    return request;
}
