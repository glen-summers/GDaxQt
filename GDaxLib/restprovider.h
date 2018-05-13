#ifndef RESTPROVIDER_H
#define RESTPROVIDER_H

#include "defs.h"
#include "trade.h"
#include "candle.h"

#include "flogging.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

class RestProvider : public QObject
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<RestProvider>();

    Q_OBJECT

    QNetworkAccessManager manager;

public:
    void fetchTrades();
    void fetchCandles();

signals:
    void error();
    void candles(std::vector<Candle> values);
    void trades(std::vector<Trade> values);

private slots:
    void error(QNetworkReply::NetworkError error);
    void sslErrors(QList<QSslError> errors);
    void candlesFinished(QNetworkReply * reply);
    void tradesFinished(QNetworkReply * reply);
};

#endif // RESTPROVIDER_H
