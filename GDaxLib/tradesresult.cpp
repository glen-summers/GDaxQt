#include "tradesresult.h"

#include <QJsonDocument>
#include <QJsonArray>

TradesResult::TradesResult(QNetworkReply * reply)
    : Result(reply)
    , array(HasError() ? QJsonArray() : QJsonDocument::fromJson(reply->readAll()).array())
{
}

TradesResult::iterator TradesResult::begin() const
{
    return {array, 0};
}

const TradesResult::iterator TradesResult::end() const
{
    return {array, array.size()};
}
