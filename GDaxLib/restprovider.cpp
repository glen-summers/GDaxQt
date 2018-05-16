
#include "restprovider.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValueRef>
#include <QMetaEnum>
#include <QNetworkCookieJar>
#include <QUrlQuery>

void RestProvider::FetchAllCandles(Granularity granularity)
{
    QUrlQuery query;
    query.addQueryItem("granularity", QString::number(static_cast<unsigned int>(granularity)));

    QUrl url(QString(Url).arg(Product, Candles));
    url.setQuery(query);

    log.Info(QString("Requesting %1").arg(url.toString()));
    QNetworkRequest request(url);
    QNetworkReply * reply = manager.get(request);
    // reply->ignoreSslErrors();// allows fidler, set in cfg
    connect(reply, &QNetworkReply::sslErrors, this, &RestProvider::SslErrors);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &RestProvider::Error);
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

    QUrl url(QString(Url).arg(Product, Candles));
    url.setQuery(query);

    log.Info(QString("Requesting %1").arg(url.toString()));
    QNetworkRequest request(url);
    QNetworkReply * reply = manager.get(request);
    // reply->ignoreSslErrors();// allows fidler, set in cfg
    connect(reply, &QNetworkReply::sslErrors, this, &RestProvider::SslErrors);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &RestProvider::Error);
    connect(reply, &QNetworkReply::finished, [this, reply]() { RestProvider::CandlesFinished(reply); });
}

void RestProvider::FetchTrades(unsigned int limit)
{
    QUrlQuery query;
    query.addQueryItem("limit", QString::number(limit));

    QUrl url(QString(Url).arg(Product, Trades));
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply * reply = manager.get(request);

    connect(reply, &QNetworkReply::sslErrors, this, &RestProvider::SslErrors);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &RestProvider::Error);
    connect(reply, &QNetworkReply::finished, [this, reply]() { RestProvider::TradesFinished(reply); });
}

void RestProvider::Error(QNetworkReply::NetworkError error)
{
    log.Error(QString("SocketError: %1").arg(QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error)));
}

void RestProvider::SslErrors(QList<QSslError> errors)
{
    for(auto & e : errors)
    {
        log.Error(QString("SslError: %1, %2").arg(e.error()).arg(e.errorString()));
    }
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

    log.Info(QString("candles %1").arg(candles.size()));

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
