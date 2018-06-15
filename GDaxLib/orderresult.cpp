#include "orderresult.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>

OrderResult OrderResult::FromReply(QNetworkReply *reply)
{
    auto error = reply->error();
    QString errorString;
    std::vector<Order> orders;

    if (error != QNetworkReply::NoError)
    {
        errorString = reply->errorString();
    }
    else
    {
        // todo keep and expose iterator to data
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        const auto & array  = document.array();
        for (const QJsonValue & t : array)
        {
            orders.push_back(Order::FromJson(t.toObject()));
        }
    }

    return {error, errorString, orders};
}
