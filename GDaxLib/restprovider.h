#ifndef RESTPROVIDER_H
#define RESTPROVIDER_H

#include "defs.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

struct Candle
{
    time_t startTime;
    Decimal lowestPrice;
    Decimal highestPrice;
    Decimal openingPrice;
    Decimal closingPrice;
    Decimal volume;
};

class RestProvider : public QObject
{
    Q_OBJECT

    QNetworkAccessManager manager;

public:
    RestProvider();

signals:
    void error();
    void data(std::vector<Candle> candles);

private slots:
    void error(QNetworkReply::NetworkError error);
    void sslErrors(QList<QSslError> errors);
    void downloadFinished(QNetworkReply * reply);
};

#endif // RESTPROVIDER_H
