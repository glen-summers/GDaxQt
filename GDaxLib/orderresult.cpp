#include "orderresult.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>

OrderResult::OrderResult(QNetworkReply *reply)
    : Result(reply)
    , array(HasError() ? QJsonArray() : QJsonDocument::fromJson(reply->readAll()).array())
{
}

OrderResult::Iterator OrderResult::begin() const
{
    return {array, 0};
}

const OrderResult::Iterator OrderResult::end() const
{
    return {array, array.size()};
}

OrderResult::Iterator::Iterator(const QJsonArray &array, int index)
    : array(array), index(index), order()
{
    if (index!=array.size())
    {
        order = Order::FromJson(array[index].toObject());
    }
}

OrderResult::Iterator &OrderResult::Iterator::operator++()
{
    ++index;
    if (index!=array.size())
    {
        order = Order::FromJson(array[index].toObject());
    }
    else
    {
        order = {};
    }
    return *this;
}
