#ifndef CANDLESRESULT_H
#define CANDLESRESULT_H

#include "result.h"
#include "candle.h"
#include "jsonarrayiterator.h"

#include <QNetworkReply>
#include <QJsonArray>

class CandlesResult : public Result
{
    QJsonArray const array;

public:
    typedef JsonArrayIterator<Candle> iterator;
    CandlesResult(QNetworkReply * reply);

    iterator begin() const;
    const iterator end() const;
};

#endif // CANDLESRESULT_H
