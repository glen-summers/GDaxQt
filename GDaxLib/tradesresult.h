#ifndef TRADESRESULT_H
#define TRADESRESULT_H

#include "result.h"
#include "trade.h"
#include "jsonarrayiterator.h"

#include <QNetworkReply>
#include <QJsonArray>

class TradesResult : public Result
{
    QJsonArray const array;

public:
    typedef JsonArrayIterator<Trade> iterator;
    TradesResult(QNetworkReply * reply);

    iterator begin() const;
    const iterator end() const;
};

#endif // TRADESRESULT_H
