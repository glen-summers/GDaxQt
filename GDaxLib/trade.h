#ifndef TRADE_H
#define TRADE_H

#include "defs.h"

#include <QJsonObject>
#include <QDateTime>

// merge trade\tick - are they the same? trade missing sequence number
class Trade
{
public:
    typedef unsigned long long TradeId;

    static Trade FromJson(const QJsonObject & object);

    QDateTime time;
    TradeId tradeId;
    Decimal price;
    Decimal size;
    MakerSide side;
};

#endif // TRADE_H
