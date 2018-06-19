#ifndef RESTPROVIDER_H
#define RESTPROVIDER_H

#include "gdl.h" // avoid here?

#include "decimalwrap.h"

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
    std::unique_ptr<Authenticator> authenticator;

public:
    RestProvider(const char * baseUrl, QNetworkAccessManager * manager, QObject * parent = nullptr);
    ~RestProvider();

    void SetAuthentication(QByteArray apiKey, QByteArray secretKey, QByteArray passphrase);

    void ClearAuthentication();

    Async<ServerTimeResult> FetchTime();

    void FetchTrades(std::function<void(TradesResult)> func, unsigned int limit);

    void FetchAllCandles(std::function<void(CandlesResult)> func, Granularity granularity);

    void FetchCandles(std::function<void(CandlesResult)> func, const QDateTime & start,
                      const QDateTime & end, Granularity granularity);

    Async<OrdersResult> FetchOrders(unsigned int limit = 0);

    Async<OrderResult> PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side);

    Async<CancelOrdersResult> CancelOrders();

private:
    QNetworkRequest CreateAuthenticatedRequest(const QString & httpMethod, const QString & requestPath, const QUrlQuery & query,
                                               const QByteArray & body) const;

    QNetworkRequest CreateRequest(const QString & requestPath, const QUrlQuery & query) const;

};

#endif // RESTPROVIDER_H
