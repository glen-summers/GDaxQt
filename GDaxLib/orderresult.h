#ifndef ORDERRESULT_H
#define ORDERRESULT_H

#include "order.h"

#include <QString>
#include <QNetworkReply>

#include <vector> // avoid

struct OrderResult
{
    QNetworkReply::NetworkError error;
    QString errorString;
    std::vector<Order> orders;

    static OrderResult FromReply(QNetworkReply * reply);
};

#endif // ORDERRESULT_H
