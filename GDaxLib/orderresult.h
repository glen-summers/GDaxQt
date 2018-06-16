#ifndef ORDERRESULT_H
#define ORDERRESULT_H

#include "result.h"
#include "order.h"
#include "jsonarrayiterator.h"

#include <QNetworkReply>
#include <QJsonArray>

class OrdersResult : public Result
{
    QJsonArray const array;

public:
    typedef JsonArrayIterator<Order> iterator;
    OrdersResult(QNetworkReply * reply);

    iterator begin() const;
    const iterator end() const;
};

#endif // ORDERRESULT_H
