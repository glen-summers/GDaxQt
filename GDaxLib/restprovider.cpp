#include "restprovider.h"

#include "utils.h"
#include "authenticator.h"

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

    constexpr const char CbAccessKey[]= "CB-ACCESS-KEY";
    constexpr const char CbAccessSign[] = "CB-ACCESS-SIGN";
    constexpr const char CbAccessTimestamp[] = "CB-ACCESS-TIMESTAMP";
    constexpr const char CbAccessPassphrase[] = "CB-ACCESS-PASSPHRASE";

    // informational atm
    void Error(QNetworkReply::NetworkError error)
    {
        flog.Error("SocketError: {0}", QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error));
    }

    void SslErrors(QList<QSslError> errors)
    {
        for (auto & e : errors)
        {
            flog.Error("SslError: {0}, {1}", e.error(), e.errorString().toStdString());
        }
    }
}

RestProvider::RestProvider(const char * baseUrl, QNetworkAccessManager * manager, QObject * parent)
    : QObject(parent)
    , baseUrl(baseUrl)
    , manager(manager)
    , authenticator()
{
}

void RestProvider::SetAuthenticator(Authenticator * newAuthenticator)
{
    authenticator = newAuthenticator;
}

void RestProvider::FetchAllCandles(Granularity granularity)
{
    QUrlQuery query;
    query.addQueryItem("granularity", QString::number(static_cast<unsigned int>(granularity)));

    QUrl url(baseUrl % Products % Product % Candles);
    url.setQuery(query);

    flog.Info("Requesting {0}", url.toString());
    QNetworkRequest request(url);
    QNetworkReply * reply = manager->get(request);
    // reply->ignoreSslErrors();// allows fidler, set in cfg
    connect(reply, &QNetworkReply::sslErrors, &SslErrors);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), &Error);
    connect(reply, &QNetworkReply::finished, [this, reply]() { RestProvider::CandlesFinished(reply); });
}

void RestProvider::FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity)
{
    // this version seems flakey on the server, if we request with an endtime > server UTC time then
    // both parameters are ignored and the last 300 candles are returned

    QUrlQuery query;
    query.addQueryItem("start", start.toString(Qt::ISODate));
    query.addQueryItem("end", end.toString(Qt::ISODate));
    query.addQueryItem("granularity", QString::number(static_cast<unsigned int>(granularity)));

    QUrl url(baseUrl % Products % Product % Candles);
    url.setQuery(query);

    flog.Info("Requesting {0}", url.toString());
    QNetworkRequest request(url);
    QNetworkReply * reply = manager->get(request);
    // reply->ignoreSslErrors();// allows fidler, set in cfg
    connect(reply, &QNetworkReply::sslErrors, &SslErrors);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), &Error);
    connect(reply, &QNetworkReply::finished, [this, reply]() { RestProvider::CandlesFinished(reply); });
}

void RestProvider::FetchOrders(unsigned int limit, std::function<void (OrderResult)> func)
{
    if (!authenticator)
    {
        throw std::runtime_error("Method requires authentication");
    }

    QUrlQuery query;
    if (limit != 0)
    {
        query.addQueryItem("limit", QString::number(limit));
    }

    QNetworkRequest request = CreateAuthenticatedRequest("GET", Orders, query, {});
    QNetworkReply * reply = manager->get(request); // reply always deleted? finished may not get called, always hook error?
    reply->ignoreSslErrors();// allows fidler, set in cfg
    //connect(reply, &QNetworkReply::sslErrors, &SslErrors);
    //connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), &Error);
    connect(reply, &QNetworkReply::finished, [func{std::move(func)}, reply]()
    {
        func(OrderResult::FromReply(reply));
        reply->deleteLater();
    });
}

void RestProvider::PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side)
{
    if (!authenticator)
    {
        throw std::runtime_error("Method requires authentication");
    }
    assert(side != MakerSide::None);
    QString siderian = side == MakerSide::Buy ? "buy":"sell";

    QString sz = DecNs::toString(size).c_str();
    QString pr = DecNs::toString(price).c_str();

    QJsonDocument doc(QJsonObject
    {
        {"price", pr},
        {"size", sz},
        {"side", siderian },
        {"product_id", Product}
    });
    auto data = doc.toJson();

    QNetworkRequest request = CreateAuthenticatedRequest("POST", Orders, {}, data);
    // agent?
    request.setRawHeader("Content-Type", "application/json");

    QNetworkReply * reply = manager->post(request, data);
    reply->ignoreSslErrors();// allows fidler, set in cfg
    connect(reply, &QNetworkReply::sslErrors, &SslErrors);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), &Error);
    connect(reply, &QNetworkReply::finished, [&]()
    {
        flog.Info("OrderFinished");
        // return order via lambda
    });
}

void RestProvider::CancelOrders()
{
    if (!authenticator)
    {
        throw std::runtime_error("Method requires authentication");
    }

    // + product_id
    QNetworkRequest request = CreateAuthenticatedRequest("DELETE", Orders, {}, {});
    QNetworkReply * reply = manager->deleteResource(request);
    reply->ignoreSslErrors();// allows fidler, set in cfg
    connect(reply, &QNetworkReply::sslErrors, &SslErrors);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), &Error);
    connect(reply, &QNetworkReply::finished, [&]()
    {
        flog.Info("Delete Orders Finished");
        // return result via lambda
    });
}

void RestProvider::FetchTrades(unsigned int limit)
{
    QUrlQuery query;
    query.addQueryItem("limit", QString::number(limit));

    QUrl url(baseUrl % Products % Product % Trades);
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply * reply = manager->get(request);

    connect(reply, &QNetworkReply::sslErrors, &SslErrors);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), &Error);
    connect(reply, &QNetworkReply::finished, [this, reply]() { RestProvider::TradesFinished(reply); });
}

void RestProvider::CandlesFinished(QNetworkReply *reply)
{
    if (reply->error())
    {
        emit OnError();
        return;
    }

    std::deque<Candle> candles;
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    for (const auto & a1: document.array())
    {
        const auto & ar = a1.toArray();
        time_t startTime = (time_t)ar[0].toDouble();
        Decimal lowestPrice(ar[1].toDouble());
        Decimal highestPrice(ar[2].toDouble());
        Decimal openingPrice(ar[3].toDouble());
        Decimal closingPrice(ar[4].toDouble());
        Decimal volume(ar[5].toDouble());

      candles.push_back({startTime, lowestPrice, highestPrice, openingPrice, closingPrice, volume});
    }
    reply->deleteLater();

    flog.Info("candles {0}", candles.size());

    emit OnCandles(std::move(candles));
}

void RestProvider::TradesFinished(QNetworkReply *reply)
{
    if (reply->error())
    {
        emit OnError();
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    std::deque<Trade> trades;
    const auto & array  = document.array();
    for (const QJsonValue & t : array)
    {
        trades.push_back(Trade::FromJson(t.toObject()));
    }
    reply->deleteLater();

    emit OnTrades(std::move(trades));
}

QNetworkRequest RestProvider::CreateAuthenticatedRequest(const QString & httpMethod, const QString & requestPath, const QUrlQuery & query,
                                                         const QByteArray & body) const
{
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

    // just return manager->get(request);
    return request;
}
