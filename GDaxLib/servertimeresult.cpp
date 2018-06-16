#include "servertimeresult.h"
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
    QDateTime GetTime(const QJsonDocument &doc)
    {
        return QDateTime::fromString(doc.object()["iso"].toString(), Qt::DateFormat::ISODateWithMs);
    }
}

ServerTimeResult::ServerTimeResult(QNetworkReply *reply)
    : Result(reply)
    , value(HasError() ? QDateTime() : GetTime(QJsonDocument::fromJson(reply->readAll())))
{
}

