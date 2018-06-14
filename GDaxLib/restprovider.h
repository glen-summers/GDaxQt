#ifndef RESTPROVIDER_H
#define RESTPROVIDER_H

#include "trade.h"
#include "candle.h"
#include "order.h"

#include <QObject>
#include <QNetworkRequest>

#include <deque>

class Authenticator;

class QString;
class QNetworkAccessManager;
class QNetworkReply;
class QUrlQuery;

class RestProvider : public QObject
{
    Q_OBJECT

    QString const baseUrl;
    QNetworkAccessManager * const manager;
    Authenticator * authenticator;

public:
    RestProvider(const char * baseUrl, QNetworkAccessManager * manager, QObject * parent = nullptr);

    void SetAuthenticator(Authenticator * authenticator);

    void FetchTrades(unsigned int limit);
    void FetchAllCandles(Granularity granularity);
    void FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity);

    void FetchOrders(unsigned int limit = 0);
    void PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side);

signals:
    void OnError(); // make more specific

    // cld try passing a deserializer\iterators to avoid forcing container types
    void OnCandles(std::deque<Candle> values);
    void OnTrades(std::deque<Trade> values);
    void OnOrders(std::vector<Order> values);

private slots:
    void CandlesFinished(QNetworkReply * reply);
    void TradesFinished(QNetworkReply * reply);
    void OrdersFinished(QNetworkReply *reply);

private:
    QNetworkRequest CreateAuthenticatedRequest(const QString & httpMethod, const QString & requestPath, const QUrlQuery & query,
                                               const QByteArray & body) const;
};

#endif // RESTPROVIDER_H
