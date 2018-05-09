
#include "restprovider.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValueRef>

#include <deque>

RestProvider::RestProvider()
{
    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(downloadFinished(QNetworkReply*)));

    QUrl url("https://api.gdax.com/products/BTC-EUR/candles?granularity=3600");
    QNetworkRequest request(url);
    QNetworkReply * reply = manager.get(request);

    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)));
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

void RestProvider::downloadFinished(QNetworkReply *reply)
{
    if (reply->error())
    {
        emit error();
        return;
    }

    std::vector<Candle> candles;
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

      candles.push_back({startTime,  lowestPrice, highestPrice, openingPrice, closingPrice, volume});
    }
    reply->deleteLater();

    emit data(std::move(candles));
}
