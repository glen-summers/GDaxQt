#include "orderresult.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>

OrderResult OrderResult::FromReply(QNetworkReply *reply)
{
    auto error = reply->error();
    QString errorString;
    QJsonArray array;

    if (error != QNetworkReply::NoError)
    {
        errorString = reply->errorString();
    }
    else
    {
        array = QJsonDocument::fromJson(reply->readAll()).array();
    }

    return {error, errorString, array};
}

OrderResult::OrderResult(QNetworkReply::NetworkError error, QString errorString, QJsonArray array)
    : error(error), errorString(errorString), array(array)
{
}

QNetworkReply::NetworkError OrderResult::Error() const
{
    return error;
}

const QString &OrderResult::ErrorString() const
{
    return errorString;
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
