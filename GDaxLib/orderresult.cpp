#include "orderresult.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>

OrdersResult::OrdersResult(QNetworkReply *reply)
    : Result(reply)
    , array(HasError() ? QJsonArray() : QJsonDocument::fromJson(reply->readAll()).array())
{
}

OrdersResult::iterator OrdersResult::begin() const
{
    return {array, 0};
}

const OrdersResult::iterator OrdersResult::end() const
{
    return {array, array.size()};
}
