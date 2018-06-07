#ifndef RESTPROVIDER_H
#define RESTPROVIDER_H

#include "trade.h"
#include "candle.h"

#include <QObject>

#include <deque>

class QNetworkAccessManager;
class QNetworkReply;

class RestProvider : public QObject
{
    Q_OBJECT

    QString const productUrl;
    QNetworkAccessManager * const manager;

public:
    RestProvider(const char * baseUrl, QNetworkAccessManager * manager, QObject * parent = nullptr);

    void FetchTrades(unsigned int limit);
    void FetchAllCandles(Granularity granularity);
    void FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity);

signals:
    void OnError(); // make more specific
    void OnCandles(std::deque<Candle> values);
    void OnTrades(std::deque<Trade> values);

private slots:
    void CandlesFinished(QNetworkReply * reply);
    void TradesFinished(QNetworkReply * reply);
};

#endif // RESTPROVIDER_H
