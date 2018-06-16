#include "candle.h"

#include <QJsonValue>

Candle Candle::FromJson(const QJsonValue & value)
{
    time_t startTime = (time_t)value[0].toDouble();
    Decimal lowestPrice(value[1].toDouble());
    Decimal highestPrice(value[2].toDouble());
    Decimal openingPrice(value[3].toDouble());
    Decimal closingPrice(value[4].toDouble());
    Decimal volume(value[5].toDouble());

    return {startTime, lowestPrice, highestPrice, openingPrice, closingPrice, volume};
}
