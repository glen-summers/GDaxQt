#include "result.h"

#include <QMetaEnum>

Result::Result(QNetworkReply *reply)
    : error(reply->error())
    , errorString(reply->errorString())
{}

QString Result::ErrorString() const
{
    return QString("%1 : %2").arg(errorString).arg(QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error));
}
