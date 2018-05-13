
#include "restprovider.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValueRef>

// rework as a basic request response handler
// "One QNetworkAccessManager should be enough for the whole Qt application."

void RestProvider::fetchCandles()
{
    QUrl url("https://api.gdax.com/products/BTC-EUR/candles?granularity=3600");
    QNetworkRequest request(url);
    QNetworkReply * reply = manager.get(request);
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)));
    connect(reply, &QNetworkReply::finished, [this, reply]() { RestProvider::candlesFinished(reply); });
}

void RestProvider::fetchTrades()
{
    // parameterise fetch? want to be >= trade history window rows
    QUrl url("https://api.gdax.com/products/BTC-EUR/trades?limit=100");
    QNetworkRequest request(url);
    QNetworkReply * reply = manager.get(request);
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)));
    connect(reply, &QNetworkReply::finished, [this, reply]() { RestProvider::tradesFinished(reply); });
}

void RestProvider::error(QNetworkReply::NetworkError error)
{
    // ContentNotFoundError 203
    qWarning() << QString("RestProvider::error %1)").arg(error);
}

void RestProvider::sslErrors(QList<QSslError> errors)
{
    for(auto & e : errors)
    {
        qWarning() << e.errorString();
    }
}

void RestProvider::candlesFinished(QNetworkReply *reply)
{
    if (reply->error())
    {
        qWarning() << reply->error();
        emit error();
        return;
    }

    std::vector<Candle> vec;
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

      vec.push_back({startTime, lowestPrice, highestPrice, openingPrice, closingPrice, volume});
    }
    reply->deleteLater();

    emit candles(std::move(vec));
}

void RestProvider::tradesFinished(QNetworkReply *reply)
{
    if (reply->error())
    {
        qWarning() << reply->error();
        emit error();
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    std::vector<Trade> vec;
    const auto & array  = document.array();
    vec.reserve(array.size());
    for (const QJsonValue & t : array)
    {
        vec.push_back(Trade::fromJson(t.toObject()));
    }
    reply->deleteLater();

    emit trades(std::move(vec));
}

