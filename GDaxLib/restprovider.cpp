
#include "restprovider.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValueRef>

// rework as a basic request response handler
// "One QNetworkAccessManager should be enough for the whole Qt application."

void RestProvider::FetchCandles()
{
    QUrl url("https://api.gdax.com/products/BTC-EUR/candles?granularity=3600");
    QNetworkRequest request(url);
    QNetworkReply * reply = manager.get(request);

    connect(reply, &QNetworkReply::sslErrors, this, &RestProvider::SslErrors);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &RestProvider::Error);
    connect(reply, &QNetworkReply::finished, [this, reply]() { RestProvider::CandlesFinished(reply); });
}

void RestProvider::FetchTrades()
{
    // parameterise fetch? want to be >= trade history window rows
    QUrl url("https://api.gdax.com/products/BTC-EUR/trades?limit=100");
    QNetworkRequest request(url);
    QNetworkReply * reply = manager.get(request);

    connect(reply, &QNetworkReply::sslErrors, this, &RestProvider::SslErrors);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &RestProvider::Error);
    connect(reply, &QNetworkReply::finished, [this, reply]() { RestProvider::TradesFinished(reply); });
}

void RestProvider::Error(QNetworkReply::NetworkError error)
{
    // ContentNotFoundError 203
    qWarning() << QString("RestProvider::error %1)").arg(error);
}

void RestProvider::SslErrors(QList<QSslError> errors)
{
    for(auto & e : errors)
    {
        qWarning() << e.errorString();
    }
}

void RestProvider::CandlesFinished(QNetworkReply *reply)
{
    if (reply->error())
    {
        qWarning() << reply->error();
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

    emit OnCandles(std::move(candles));
}

void RestProvider::TradesFinished(QNetworkReply *reply)
{
    if (reply->error())
    {
        qWarning() << reply->error();
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
