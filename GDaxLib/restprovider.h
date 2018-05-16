#ifndef RESTPROVIDER_H
#define RESTPROVIDER_H

#include "defs.h"
#include "trade.h"
#include "candle.h"

#include "flogging.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <deque>

class RestProvider : public QObject
{
    inline static const Flog::Log log = Flog::LogManager::GetLog<RestProvider>();

    static constexpr const char Url[] = "https://api.gdax.com/products/%1/%2";
    static constexpr const char Product[] = "BTC-EUR";
    static constexpr const char Candles[] = "candles";
    static constexpr const char Trades[] = "trades";

    Q_OBJECT

    QNetworkAccessManager manager;

public:
    void FetchTrades(unsigned int limit);
    void FetchAllCandles(Granularity granularity);
    void FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity);

signals:
    void OnError();
    void OnCandles(std::deque<Candle> values);
    void OnTrades(std::deque<Trade> values);

private slots:
    void Error(QNetworkReply::NetworkError error);
    void SslErrors(QList<QSslError> errors);
    void CandlesFinished(QNetworkReply * reply);
    void TradesFinished(QNetworkReply * reply);
};

#endif // RESTPROVIDER_H
