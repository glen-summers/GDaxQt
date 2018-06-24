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

class RestProvider : public QObject, public GDL::IRequest
{
    Q_OBJECT

    QString const baseUrl;
    QNetworkAccessManager * const manager;
    std::unique_ptr<Authenticator> authenticator;

public:
    RestProvider(const char * baseUrl, QObject * parent = nullptr);
    ~RestProvider();

    void SetAuthentication(const char key[], const char secret[], const char passphrase[]) override;

    void ClearAuthentication() override;

    Async<ServerTimeResult> FetchTime();

    Async<TradesResult> FetchTrades(unsigned int limit);

    Async<CandlesResult> FetchAllCandles(Granularity granularity);

    Async<CandlesResult> FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity);

    Async<OrdersResult> FetchOrders(unsigned int limit = 0);

    Async<OrderResult> PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side);

    Async<CancelOrdersResult> CancelOrders();

private:
    QNetworkRequest CreateAuthenticatedRequest(const QString & httpMethod, const QString & requestPath, const QUrlQuery & query,
                                               const QByteArray & body) const;

    QNetworkRequest CreateRequest(const QString & requestPath, const QUrlQuery & query) const;

};

#endif // RESTPROVIDER_H
