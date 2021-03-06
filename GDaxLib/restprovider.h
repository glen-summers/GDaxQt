#ifndef RESTPROVIDER_H
#define RESTPROVIDER_H

#include "gdl.h" // avoid here?

#include "decimalwrap.h"

#include <QObject>
#include <QNetworkReply>
#include <functional>

class Authenticator;

class QString;
class QNetworkAccessManager;
class QUrlQuery;

class RestProvider : public QObject, public GDL::IRequest
{
    Q_OBJECT

    enum class RequestMethod {Get, Post, Delete};

    QString const baseUrl;
    QString const product;
    std::unique_ptr<Authenticator> const authenticator;
    QNetworkAccessManager * const manager;

public:
    RestProvider(const char * baseUrl, const char * product, GDL::Auth * auth);
    ~RestProvider();

    Async<ServerTimeResult> FetchTime();

    Async<TradesResult> FetchTrades(unsigned int limit);

    Async<CandlesResult> FetchAllCandles(Granularity granularity);

    Async<CandlesResult> FetchCandles(const QDateTime & start, const QDateTime & end, Granularity granularity);

    Async<OrdersResult> FetchOrders(unsigned int limit = 0);

    Async<OrderResult> PlaceOrder(const Decimal & size, const Decimal & price, MakerSide side);

    Async<CancelOrdersResult> CancelOrders();

private:
    void Authenticate(QNetworkRequest & request, const QString & httpMethod, const QByteArray * body) const;

    QNetworkReply * CreateRequest(bool authenicate, RequestMethod method, const QString & requestPath,
                                  const QUrlQuery * query = nullptr, const QByteArray * body = nullptr) const;

    static QString RequestMethodToString(RequestMethod method);
};

#endif // RESTPROVIDER_H
