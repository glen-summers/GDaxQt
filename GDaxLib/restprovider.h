#ifndef RESTPROVIDER_H
#define RESTPROVIDER_H

#include "trade.h"
#include "candle.h"
#include "order.h"
#include "tradesresult.h"
#include "candlesresult.h"
#include "orderresult.h"
#include "servertimeresult.h"

#include <QObject>
#include <QNetworkRequest>

#include <functional>

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

    void FetchTime(std::function<void(ServerTimeResult)> func);

    void FetchTrades(std::function<void(TradesResult)> func, unsigned int limit);

    void FetchAllCandles(std::function<void(CandlesResult)> func, Granularity granularity);

    void FetchCandles(std::function<void(CandlesResult)> func, const QDateTime & start,
                      const QDateTime & end, Granularity granularity);

    void FetchOrders(std::function<void(OrdersResult)> func, unsigned int limit = 0);

    void PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side);

    void CancelOrders();

private:
    QNetworkRequest CreateAuthenticatedRequest(const QString & httpMethod, const QString & requestPath, const QUrlQuery & query,
                                               const QByteArray & body) const;

    QNetworkRequest CreateRequest(const QString & requestPath, const QUrlQuery & query) const;

};

#endif // RESTPROVIDER_H
