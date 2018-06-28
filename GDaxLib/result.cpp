#include "result.h"

#include <QMetaEnum>
#include <QNetworkReply>

Result::Result(QNetworkReply *reply)
    : error(reply ? reply->error() : QNetworkReply::NetworkError::NoError) // avoid one error?
    , errorString(reply ? reply->errorString() : "")//
{}

std::string Result::ErrorString() const
{
    return QString("%1 : %2").arg(errorString).arg(QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error)).toStdString();
}

void Result::Verify() const
{
    if (HasError())
    {
        throw std::runtime_error(ErrorString());
    }
}
