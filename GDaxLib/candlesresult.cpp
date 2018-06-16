#include "candlesresult.h"

#include <QJsonDocument>

CandlesResult::CandlesResult(QNetworkReply *reply)
    : Result(reply)
    , array(HasError() ? QJsonArray() : QJsonDocument::fromJson(reply->readAll()).array())
{
}

CandlesResult::iterator CandlesResult::begin() const
{
    return {array, 0};
}

const CandlesResult::iterator CandlesResult::end() const
{
    return {array, array.size()};
}
